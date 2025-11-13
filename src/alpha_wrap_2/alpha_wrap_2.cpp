#include <alpha_wrap_2/alpha_wrap_2.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>


namespace aw2 {

    alpha_wrap_2::alpha_wrap_2(const Oracle& oracle) 
        : oracle_(oracle), 
          traversability_(nullptr) {}

    alpha_wrap_2::~alpha_wrap_2() {
        delete traversability_;
    }

    void alpha_wrap_2::run() {

        namespace fs = std::filesystem;

        auto style = StyleConfig::preset_style();
        alpha_wrap_2_exporter exporter(*this, style);
        exporter.setup_export_dir(config_.output_directory);
        
        total_timer_->start();
        main_loop_timer_->start();

        int iteration = 0;
        while (!queue_.empty()) {
            
            if (++iteration > max_iterations_) {
                std::cout << "Reached maximum number of iterations (" << max_iterations_ << "). Stopping." << std::endl;
                break;
            }
            std::cout << "\nIteration: " << iteration << " Queue size: " << queue_.size() << std::endl;

            // ** Get candidate gate **
            candidate_gate_ = queue_.top();
            queue_.pop();

            if ((iteration % config_.intermediate_steps) == 0 && (iteration < config_.export_step_limit)) {
                exporter.export_svg("in_progress_iter_" + std::to_string(iteration) + ".svg");
            }

            // ** Get candidate gate info **
            gate_processing_timer_->start();

            auto info = gate_adjacency_info(candidate_gate_.edge);
            auto c_in = candidate_gate_.edge.first;
            auto c_in_cc = info.cc_inside;
            auto c_out_cc = info.cc_outside;

            gate_processing_timer_->pause();

            // ** Process rule 1 **
            if (process_rule_1(c_in_cc, c_out_cc)) {
                statistics_.execution_stats.n_rule_1++;
                std::cout << "Steiner point inserted by R1." << std::endl;
                continue;
            }

            // ** Process rule 2 **
            if (process_rule_2(c_in, c_in_cc)) {
                statistics_.execution_stats.n_rule_2++;
                std::cout << "Steiner point inserted by R2." << std::endl;
                continue;
            }

            // ** Carve face **
            std::cout << "No Steiner point inserted. Marking c_in as OUTSIDE." << std::endl;
            c_in->info() = OUTSIDE;
            update_queue(c_in);
        }

        main_loop_timer_->pause();

        // ** Extract wrap surface **
        extraction_timer_->start();
        extract_wrap_surface();
        extraction_timer_->pause();

        total_timer_->pause();
        
        // Export result and collect statistics
        exporter.export_svg("final_result.svg");

        statistics_.execution_stats.n_iterations = iteration;
        statistics_.timings.total_time = total_timer_->elapsed_ms();
        statistics_.timings.gate_processing = gate_processing_timer_->elapsed_ms();
        statistics_.timings.rule_1_processing = rule1_timer_->elapsed_ms();
        statistics_.timings.rule_2_processing = rule2_timer_->elapsed_ms();
        
        statistics_.output_stats.n_vertices = dt_.number_of_vertices();
        statistics_.output_stats.n_edges = wrap_edges_.size();
        
        // Get current timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::ostringstream timestamp_oss;
        timestamp_oss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        statistics_.metadata.timestamp = timestamp_oss.str();
        
        // Export statistics to JSON
        std::string stats_filepath = exporter.export_dir_.string() + "/statistics.json";
        statistics_.export_to_json(stats_filepath);
        std::cout << "\nStatistics exported to: " << stats_filepath << std::endl;
        
        // Print hierarchical timing report
        registry_.print_all_hierarchies();
        std::cout << "Total iterations: " << iteration << std::endl;
    }


    void alpha_wrap_2::init(AlgorithmConfig& config) {

        // Create hierarchical timer structure
        total_timer_ = registry_.create_root_timer("Alpha Wrap Algorithm");
        init_timer_ = total_timer_->create_child("Initialization");
        main_loop_timer_ = total_timer_->create_child("Main Loop");
        rule1_timer_ = main_loop_timer_->create_child("Rule 1 Processing");
        rule2_timer_ = main_loop_timer_->create_child("Rule 2 Processing");
        gate_processing_timer_ = main_loop_timer_->create_child("Gate Processing");
        extraction_timer_ = total_timer_->create_child("Extraction");

        total_timer_->start();
        init_timer_->start();

        // apply configuration
        alpha_ = config.alpha;
        offset_ = config.offset;
        max_iterations_ = config.max_iterations;
        config_ = config;

        // Populate config stats
        statistics_.config.alpha = config.alpha;
        statistics_.config.offset = config.offset;
        statistics_.config.traversability_params = config.traversability_params;
        
        // Set traversability object
        switch (config.traversability_method) {
            case CONSTANT_ALPHA:
                traversability_ = new ConstantAlphaTraversability(alpha_);
                statistics_.config.traversability_function = "CONSTANT_ALPHA";
                break;
            case DEVIATION_BASED:
                traversability_ = new DeviationBasedTraversability(
                    alpha_, 
                    offset_, 
                    oracle_,
                    std::get<DeviationBasedParams>(config.traversability_params)
                );
                statistics_.config.traversability_function = "DEVIATION_BASED";
                
                break;
            case INTERSECTION_BASED:
                traversability_ = new IntersectionBasedTraversability(
                    alpha_, 
                    offset_, 
                    oracle_,
                    std::get<IntersectionBasedParams>(config.traversability_params)
                );
                statistics_.config.traversability_function = "INTERSECTION_BASED";
                
                break;
            default:
                throw std::invalid_argument("Unknown traversability method.");
        }

        // Insert bounding box points
        FT margin = offset_ + bbox_diagonal_length_ / 10.0;
        auto bbox = oracle_.bbox_;
        std::vector<Point_2> pts_bbox = {
            {bbox.x_min - margin, bbox.y_min - margin}, 
            {bbox.x_min - margin, bbox.y_max + margin}, 
            {bbox.x_max + margin, bbox.y_min - margin}, 
            {bbox.x_max + margin, bbox.y_max + margin}
        };
        dt_.insert(pts_bbox.begin(), pts_bbox.end());

        // add all boundary edges to the queue and set the face labels
        for (auto eit = dt_.finite_edges_begin(); eit != dt_.finite_edges_end(); ++eit) {
            auto c_1 = eit->first;
            auto c_2 = c_1->neighbor(eit->second);
            
            if (dt_.is_infinite(c_1) && !dt_.is_infinite(c_2)) {
                c_1->info() = OUTSIDE;
                c_2->info() = INSIDE;
            }
            else if (dt_.is_infinite(c_2) && !dt_.is_infinite(c_1)) {
                c_2->info() = OUTSIDE;
                c_1->info() = INSIDE;
            }
            else {
                continue;
            }
            
            // add gate to queue
            add_gate_to_queue(*eit);
        }

        bbox_diagonal_length_ = std::sqrt(CGAL::squared_distance(
            Point_2(bbox.x_min, bbox.y_min),
            Point_2(bbox.x_max, bbox.y_max)
        ));

        init_timer_->pause();
        total_timer_->pause();
    }

    bool alpha_wrap_2::is_gate(const Delaunay::Edge& e) const {
        auto c_in = e.first;
        auto c_out = c_in->neighbor(e.second);
        return (c_in->info() != c_out->info());
    }

    EdgeAdjacencyInfo alpha_wrap_2::gate_adjacency_info(const Delaunay::Edge& edge) const {
        EdgeAdjacencyInfo info;
        info.edge = edge;
        auto c_in = edge.first;
        int i = edge.second;            
        auto c_out = c_in->neighbor(i);

        if (dt_.is_infinite(c_in)) {
            throw std::runtime_error("Error: c_in is infinite face.");
        }

        info.cc_inside = dt_.circumcenter(c_in);

        if (dt_.is_infinite(c_out)) {
            info.outside_infinite = true;
            info.cc_outside = infinite_face_cc(c_in, c_out, i);
        }
        else {
            info.outside_infinite = false;
            info.cc_outside = dt_.circumcenter(c_out);
        }

        return info;
    }


    // Return the squared radius of the minimal Delaunay ball through the edge
    FT alpha_wrap_2::sq_minimal_delaunay_ball_radius(const Gate& gate) const {

        auto adj = gate_adjacency_info(gate.edge);
        auto p1 = gate.get_points().first;
        auto p2 = gate.get_points().second;
        auto min_ball_center = CGAL::midpoint(p1, p2);
        auto sq_min_ball_radius = CGAL::squared_distance(p1, p2) / 4;

        auto sq_inside_ball_radius = CGAL::squared_distance(adj.cc_inside, p1);

        // INFINITE outer cell

        if (adj.outside_infinite) {
            if (CGAL::squared_distance(adj.cc_inside, min_ball_center) < sq_min_ball_radius) {
                return sq_inside_ball_radius;
            }
            return sq_min_ball_radius;
        }

        // FINITE outer cell

        // Case 1: minimum ball is Delaunay:

        if (CGAL::orientation(p1, p2, adj.cc_inside) != CGAL::orientation(p1, p2, adj.cc_outside)) {
            return sq_min_ball_radius;
        }

        
        // Case 2: minimum ball is not Delaunay

        auto sq_outside_ball_radius = CGAL::squared_distance(adj.cc_outside, p1);

#ifdef MODIFIED_ALPHA_TRAVERSABILITY
        
        // Case 2.1: r_in > r_out
        if (sq_outside_ball_radius < sq_inside_ball_radius) {
            std::cout << "!!! Modified alpha traversability case: " << sq_outside_ball_radius << " " << sq_inside_ball_radius << " " << sq_min_ball_radius << std::endl;
            return sq_min_ball_radius;
        }

        // Case 2.2: r_in <= r_out
        return sq_inside_ball_radius;
#else
        return std::min(sq_inside_ball_radius, sq_outside_ball_radius);
#endif
    }

    void alpha_wrap_2::update_queue(const Delaunay::Face_handle& fh){
        for(int i = 0; i < 3; ++i) {
            Delaunay::Edge e(fh, i);
            add_gate_to_queue(e);
        }
    }

    Point_2 alpha_wrap_2::infinite_face_cc(const Delaunay::Face_handle& c_in, const Delaunay::Face_handle& c_out, int edge_index) const {
        const int inf_index = c_out->index(dt_.infinite_vertex());

        // construct a circumcenter for the infinite triangle
        Point_2 p1 = c_out->vertex((inf_index + 1) % 3)->point();
        Point_2 p2 = c_out->vertex((inf_index + 2) % 3)->point();
        Point_2 mid = CGAL::midpoint(p1, p2);
        
        K::Line_2 line(p1, p2);

        // check on which side of the line the non-adjacent vertex of c_in lies
        Point_2 c_in_nonadjc = c_in->vertex(c_in->cw((edge_index+1)%3))->point();
        auto side = line.oriented_side(c_in_nonadjc);

        int sign = (side == CGAL::ON_POSITIVE_SIDE) ? -1 : 1;

        auto dir = line.perpendicular(mid).direction().to_vector();

        Point_2 far_point = mid + sign * 10000 * dir;
        return CGAL::circumcenter(p1, p2, far_point);
    }

    bool alpha_wrap_2::process_rule_1(const Point_2& c_in_cc, const Point_2& c_out_cc) {
        rule1_timer_->start();
        Point_2 steiner_point;
        bool insert = oracle_.first_intersection(
            c_out_cc,
            c_in_cc,
            steiner_point,
            offset_
        );
        if (insert) {
            rule1_timer_->pause();
            insert_steiner_point(steiner_point);
            return true;
        }
        rule1_timer_->pause();
        return false;
    }

    bool alpha_wrap_2::process_rule_2(const Delaunay::Face_handle& c_in, const Point_2& c_in_cc) {
        rule2_timer_->start();
        Point_2 steiner_point;
        auto c_in_triangle = dt_.triangle(c_in);

        if (oracle_.do_intersect(c_in_triangle)) {
            // project circumcenter onto point set
            auto p_input = oracle_.closest_point(c_in_cc);

            // insert intersection with offset surface as steiner point
            bool insert = oracle_.first_intersection(
                c_in_cc,
                p_input,
                steiner_point,
                offset_
            );
            if (insert) {
                rule2_timer_->pause();
                insert_steiner_point(steiner_point);
                return true;
            }
            else {
                throw std::runtime_error("Error: R2 failed to compute intersection point.");
            }
        }
        rule2_timer_->pause();
        return false;
    }

    void alpha_wrap_2::insert_steiner_point(const Point_2& steiner_point) {
        std::cout << "Inserting Steiner point at " << steiner_point << std::endl;
 
        // insert Steiner point
        auto vh = dt_.insert(steiner_point);

        // Update face labels
        for (auto fit = dt_.incident_faces(vh); ;) {
            if (dt_.is_infinite(fit)) {
                fit->info() = OUTSIDE;
            } else {
                fit->info() = INSIDE;
            }
            if (++fit == dt_.incident_faces(vh)) break;
        }

        gate_processing_timer_->start();
        // clear the queue
        Queue empty;
        std::swap(queue_, empty);

        // Add new gates to the queue
        for (auto eit = dt_.all_edges_begin(); eit != dt_.all_edges_end(); ++eit) {
            add_gate_to_queue(*eit);
        }
        gate_processing_timer_->pause();
    }

    void alpha_wrap_2::extract_wrap_surface() {
        // Extract edges between INSIDE and OUTSIDE faces
        wrap_edges_.clear();
        for (auto eit = dt_.finite_edges_begin(); eit != dt_.finite_edges_end(); ++eit) {
            if (is_gate(*eit)) {
                auto seg = dt_.segment(*eit);
                wrap_edges_.emplace_back(seg);
            }
        }
    }

    void alpha_wrap_2::add_gate_to_queue(const Delaunay::Edge& edge) {
        if (!is_gate(edge)) return;

        Gate g;
        auto f = edge.first;

        // orient such that INSIDE face is first
        g.edge = f->info() == INSIDE ? edge : dt_.mirror_edge(edge);
        g.sq_min_delaunay_rad = sq_minimal_delaunay_ball_radius(g);

        // add to queue if traversable
        if ((*traversability_)(g)) {
            queue_.push(g);
        }
    }

}


