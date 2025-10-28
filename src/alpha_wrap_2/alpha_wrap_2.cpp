#include <alpha_wrap_2/alpha_wrap_2.h>
#include <alpha_wrap_2/timer.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>


namespace aw2 {

    std::pair<Delaunay::Vertex_handle, Delaunay::Vertex_handle> Gate::get_vertices() const {
        auto v1 = edge.first->vertex(edge.first->cw(edge.second));
        auto v2 = edge.first->vertex(edge.first->ccw(edge.second));
        return std::make_pair(v1, v2);
    }
    std::pair<Point_2, Point_2> Gate::get_points() const {
        auto v1 = edge.first->vertex(edge.first->cw(edge.second))->point();
        auto v2 = edge.first->vertex(edge.first->ccw(edge.second))->point();
        return std::make_pair(v1, v2);
    }

    std::pair<Point_2, Point_2> EdgeAdjacencyInfo::get_points() const {
        auto v1 = edge.first->vertex(edge.first->cw(edge.second))->point();
        auto v2 = edge.first->vertex(edge.first->ccw(edge.second))->point();
        return std::make_pair(v1, v2);
    }


    alpha_wrap_2::alpha_wrap_2(const Oracle& oracle) : oracle_(oracle) {}

    void alpha_wrap_2::compute_wrap(AlgorithmConfig& config) {

        namespace fs = std::filesystem;

        // Create hierarchical timer structure
        auto& registry = TimerRegistry::instance();
        Timer* total_timer = registry.create_root_timer("Alpha Wrap Algorithm");
        Timer* init_timer = total_timer->create_child("Initialization");
        Timer* main_loop_timer = total_timer->create_child("Main Loop");
        Timer* rule1_timer = main_loop_timer->create_child("Rule 1 Processing");
        Timer* rule2_timer = main_loop_timer->create_child("Rule 2 Processing");
        Timer* gate_processing_timer = main_loop_timer->create_child("Gate Processing");
        
        total_timer->start();
        

        
        init_timer->start();
        init(config);
        init_timer->pause();

        // export config
        auto style = StyleConfig{};
        style.voronoi_diagram = {"pink", 0.6};
        style.use_gradients = true;
        style.use_opacity = true;
        style.opacity = 1.0;
        style.scheme = ColorScheme::GRADIENT;

        alpha_wrap_2_exporter exporter(*this, style);
        exporter.setup_export_dir("/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results/");


        // main loop

        std::cout << "Queue contains " << queue_.size() << " gates." << std::endl;

        int iteration = 0;
        
        main_loop_timer->start();

        while (!queue_.empty()) {
            gate_processing_timer->start();

            if (++iteration > max_iterations_) {
                std::cout << "Reached maximum number of iterations (" << max_iterations_ << "). Stopping." << std::endl;
                break;
            }

            std::cout << "\nIteration: " << iteration << " Queue size: " << queue_.size() << std::endl;

            candidate_gate_ = queue_.top();
            queue_.pop();

            
            if ((iteration % config.intermediate_steps) == 0 && (iteration < config.export_step_limit)) {
                // Export current state
                exporter.export_svg("in_progress_iter_" + std::to_string(iteration) + ".svg");
            }

            std::cout << "Candidate gate: " << candidate_gate_.get_points().first << " -- " << candidate_gate_.get_points().second << std::endl;

            if (!is_gate(candidate_gate_.edge)) {
                continue; 
            }

            if (!is_traversable_func_(candidate_gate_)) {
                continue; 
            }

            auto info = gate_adjacency_info(candidate_gate_.edge);
            auto c_in = candidate_gate_.edge.first;
            auto c_in_cc = info.cc_inside;
            auto c_out_cc = info.cc_outside;

            gate_processing_timer->pause();

            std::cout << "Dual edge: " << c_out_cc << " -> " << c_in_cc << std::endl;

            rule1_timer->start();
            if (process_rule_1(c_in_cc, c_out_cc)) {
                rule1_timer->pause();
                statistics_.execution_stats.n_rule_1++;
                std::cout << "Steiner point inserted by R1." << std::endl;
                continue;
            }
            rule1_timer->pause();

            rule2_timer->start();
            if (process_rule_2(c_in, c_in_cc)) {
                rule2_timer->pause();
                statistics_.execution_stats.n_rule_2++;
                std::cout << "Steiner point inserted by R2." << std::endl;
                continue;
            }
            rule2_timer->pause();


            std::cout << "No Steiner point inserted. Marking c_in as OUTSIDE." << std::endl;
            c_in->info() = OUTSIDE;
            update_queue(c_in);
        }

        extract_wrap_surface();

        main_loop_timer->pause();

        exporter.export_svg("final_result.svg");

        total_timer->pause();
        
        // Collect statistics
        statistics_.execution_stats.n_iterations = iteration;
        statistics_.timings.total_time = total_timer->elapsed_ms();
        statistics_.timings.gate_processing = gate_processing_timer->elapsed_ms();
        statistics_.timings.rule_1_processing = rule1_timer->elapsed_ms();
        statistics_.timings.rule_2_processing = rule2_timer->elapsed_ms();
        
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
        registry.print_all_hierarchies();
        std::cout << "Total iterations: " << iteration << std::endl;
    }


    void alpha_wrap_2::init(AlgorithmConfig& config) {

        // apply configuration
        alpha_ = config.alpha;
        offset_ = config.offset;
        max_iterations_ = config.max_iterations;

        alpha_min_ = alpha_;
        alpha_max_ = 200.0;

        // Populate config stats
        statistics_.config.alpha = config.alpha;
        statistics_.config.offset = config.offset;
        
        // Set traversability function
        switch (config.traversability_method) {
            case CONSTANT_ALPHA:
                is_traversable_func_ = [this](const Gate& g) { return is_traversable(g); };
                statistics_.config.traversability_function = "CONSTANT_ALPHA";
                break;
            case ADAPTIVE_ALPHA:
                is_traversable_func_ = [this](const Gate& g) { return is_traversable_adaptive_alpha(g); };
                statistics_.config.traversability_function = "ADAPTIVE_ALPHA";
                break;
            case DISTANCE_SAMPLING:
                is_traversable_func_ = [this](const Gate& g) { return is_traversable_dist_sampling(g); };
                statistics_.config.traversability_function = "DISTANCE_SAMPLING";
                break;
            default:
                throw std::invalid_argument("Unknown traversability method.");
        }

        // Insert bounding box points
        FT margin = 5 + offset_;
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



    }

    bool alpha_wrap_2::is_gate(const Delaunay::Edge& e) const {

        // c_in and c_out are the faces
        auto c_in = e.first;
        int i = e.second;            
        auto c_out = c_in->neighbor(i);
                    
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
    FT alpha_wrap_2::sq_minimal_delaunay_ball_radius(const Delaunay::Edge& e) const {

        auto adj = gate_adjacency_info(e);
        auto p1 = adj.get_points().first;
        auto p2 = adj.get_points().second;
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

    bool alpha_wrap_2::is_traversable(const Gate& g) const {
        return sq_minimal_delaunay_ball_radius(g.edge) >= alpha_;
    }

    bool alpha_wrap_2::is_traversable_adaptive_alpha(const Gate& g) const {
        return sq_minimal_delaunay_ball_radius(g.edge) >= std::pow(adaptive_alpha(dt_.segment(g.edge)), 2);
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
        Point_2 steiner_point;
        bool insert = oracle_.first_intersection(
            c_out_cc,
            c_in_cc,
            steiner_point,
            offset_
        );
        if (insert) {
            insert_steiner_point(steiner_point);
            return true;
        }
        return false;
    }

    bool alpha_wrap_2::process_rule_2(const Delaunay::Face_handle& c_in, const Point_2& c_in_cc) {
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
                insert_steiner_point(steiner_point);
            }
            else {
                throw std::runtime_error("Error: R2 failed to compute intersection point.");
            }
            return true;
        }
        return false;
    }

    void alpha_wrap_2::insert_steiner_point(const Point_2& steiner_point) {
        std::cout << "Inserting Steiner point at " << steiner_point << std::endl;
        // clear the queue
        Queue empty;
        std::swap(queue_, empty);
        
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

        // Add new gates to the queue
        for (auto eit = dt_.all_edges_begin(); eit != dt_.all_edges_end(); ++eit) {
            add_gate_to_queue(*eit);
        }
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

    FT alpha_wrap_2::adaptive_alpha(const Segment_2& seg) const {

        auto dev = segment_deviation(seg);
        auto adaptive_alpha = alpha_max_ * (1 - dev) + alpha_min_ * dev;
        return adaptive_alpha;
    }

    FT alpha_wrap_2::subsegment_deviation(const Segment_2& seg) const {
        auto local_pts = oracle_.local_points(seg, offset_ + 4);
        int n = local_pts.size();

        // not enough points to compute a meaningful adaptive alpha
        if (n < 5) {
            return 1.0;
        }

        // compute average squared deviation from the segment
        auto avg_sq_deviation = 0.0;
        for (const auto& pt : local_pts) {
            avg_sq_deviation += CGAL::squared_distance(seg, pt);
        }

        avg_sq_deviation /= n;
        auto dev = 0.05 * (avg_sq_deviation - std::pow(offset_, 2));
        dev = std::clamp(dev, 0.0, 1.0);

        std::cout << "dev: " << dev << " from " << avg_sq_deviation << std::endl;
        return dev;
    }

    FT alpha_wrap_2::segment_deviation(const Segment_2& seg) const {

        //auto segment_length = bbox_diagonal_length_ / 100.0;
        auto segment_length = alpha_min_;
        int m = std::ceil(std::sqrt(seg.squared_length()) / segment_length);
        auto s = seg.source();
        auto t = seg.target();

        auto avg_dev = 0.0;
        auto max_dev = 0.0;
        for (int i = 0; i < m; ++i) {
            FT t0 = static_cast<FT>(i) / m;
            FT t1 = static_cast<FT>(i + 1) / m;
            auto p0 = s + t0 * (t - s);
            auto p1 = s + t1 * (t - s);
            Segment_2 sub_seg(p0, p1);
            auto dev = subsegment_deviation(sub_seg);
            avg_dev += dev;
            if (dev > max_dev) {
                max_dev = dev;
            }
        }
        avg_dev /= m;

        // return std::clamp(avg_dev, 0.0, 1.0);
        return std::clamp(max_dev, 0.0, 1.0);
    }


    bool alpha_wrap_2::is_traversable_dist_sampling(const Gate& g) const {
        
        auto points = g.get_points();
        Point_2 s = points.first;
        Point_2 t = points.second;


        CGAL::Line_2<K> line_corr(s,t);


        Segment_2 seg(s, t);
        auto segment_length = alpha_min_;
        int m = std::ceil(std::sqrt(seg.squared_length()) / segment_length);

        for (int i = 1; i < m; ++i) {
            FT t0 = static_cast<FT>(i) / m;
            Point_2 p0 = s + t0 * (t - s);

            auto perp = line_corr.perpendicular(p0).direction().to_vector();
            Point_2 p1 = p0 + 1000*perp;

            Point_2 steiner_point;
            FT lambda;
            bool intersects = oracle_.first_intersection(
                p0,
                p1,
                steiner_point,
                offset_,
                lambda
            );
            if (intersects && CGAL::squared_distance(p0, steiner_point) > 100 * std::pow(offset_, 2)) {
                return true;
            }
            else if (!intersects) {
                return true;
            }
        }
        return false;
    }


    void alpha_wrap_2::add_gate_to_queue(const Delaunay::Edge& edge) {
        if (is_gate(edge)) {
            Gate g;
            auto f = edge.first;

            // orient such that INSIDE face is first
            if (f->info() == INSIDE) {
                g.edge = edge;
            }
            else {
                g.edge = dt_.mirror_edge(edge);
            }

            g.priority = sq_minimal_delaunay_ball_radius(g.edge);
            queue_.push(g);
        }
    }

}


