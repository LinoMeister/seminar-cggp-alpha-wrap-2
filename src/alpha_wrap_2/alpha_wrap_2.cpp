#include <alpha_wrap_2/alpha_wrap_2.h>
#include <alpha_wrap_2/timer.h>


namespace aw2 {

    std::pair<Point_2, Point_2> Gate::get_vertices() const {
        auto v1 = edge.first->vertex(edge.first->cw(edge.second))->point();
        auto v2 = edge.first->vertex(edge.first->ccw(edge.second))->point();
        return std::make_pair(v1, v2);
    }


    alpha_wrap_2::alpha_wrap_2(const Oracle& oracle) : oracle_(oracle) {}

    void alpha_wrap_2::compute_wrap(AlgorithmConfig& config) {

        namespace fs = std::filesystem;
        auto alpha = config.alpha;
        auto offset = config.offset;
        auto max_iterations = config.max_iterations;

        // setup
        std::cout << "Computing alpha-wrap-2 with " << "alpha: " << alpha << " offset: " << offset << std::endl;

        // Create hierarchical timer structure
        auto& registry = TimerRegistry::instance();
        Timer* total_timer = registry.create_root_timer("Alpha Wrap Algorithm");
        Timer* init_timer = total_timer->create_child("Initialization");
        Timer* main_loop_timer = total_timer->create_child("Main Loop");
        Timer* rule1_timer = main_loop_timer->create_child("Rule 1 Processing");
        Timer* rule2_timer = main_loop_timer->create_child("Rule 2 Processing");
        Timer* gate_processing_timer = main_loop_timer->create_child("Gate Processing");
        
        total_timer->start();
        
        alpha_ = alpha;
        offset_ = offset;
        
        init_timer->start();
        init();
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

            if (++iteration > max_iterations) {
                std::cout << "Reached maximum number of iterations (" << max_iterations << "). Stopping." << std::endl;
                break;
            }

            std::cout << "\nIteration: " << iteration << " Queue size: " << queue_.size() << std::endl;

            candidate_gate_ = queue_.top();
            queue_.pop();

            
            if ((iteration % config.intermediate_steps) == 0 && (iteration < config.export_step_limit)) {
                // Export current state
                exporter.export_svg("in_progress_iter_" + std::to_string(iteration) + ".svg");
            }

            std::cout << "Candidate gate: " << candidate_gate_.get_vertices().first << " -- " << candidate_gate_.get_vertices().second << std::endl;

            if (!is_gate(candidate_gate_.edge)) {
                continue; 
            }

            if (!is_alpha_traversable(candidate_gate_.edge, alpha)) {
                continue; 
            }


            // Traverse the gate
            auto c_1 = candidate_gate_.edge.first;
            int i = candidate_gate_.edge.second;            
            auto c_2 = c_1->neighbor(i);


            
            // Find c_in and c_out


            Delaunay::Face_handle c_in; 
            Delaunay::Face_handle c_out;

            if (c_1->info() == INSIDE && c_2->info() == OUTSIDE) {
                c_in = c_1;
                c_out = c_2;
            }
            else if (c_1->info() == OUTSIDE && c_2->info() == INSIDE) {
                c_in = c_2;
                c_out = c_1;
            }
            else {
                // This should not happen
                throw std::runtime_error("Error: Encountered gate with identical labels.");
            }

            if (dt_.is_infinite(c_in)) {
                throw std::runtime_error("Error: c_in is infinite face.");
            }

            // print vertices of c_in and c_out
            std::cout << "c_in vertices: ";
            for (int vi = 0; vi < 3; ++vi) {
                std::cout << "(" << c_in->vertex(vi)->point() << ") ";
            }
            std::cout << std::endl;
            std::cout << "c_out vertices: ";
            for (int vi = 0; vi < 3; ++vi) {
                std::cout << "(" << c_out->vertex(vi)->point() << ") ";
            }
            std::cout << std::endl;


            // compute circumcenters of c_in and c_out
            Point_2 c_in_cc = dt_.circumcenter(c_in);
            Point_2 c_out_cc;

            if (dt_.is_infinite(c_out)) {
                c_out_cc = infinite_face_cc(c_in, c_out, i);
            }
            else {
                c_out_cc = dt_.circumcenter(c_out);
            }

            gate_processing_timer->pause();

            std::cout << "Dual edge: " << c_out_cc << " -> " << c_in_cc << std::endl;

            rule1_timer->start();
            if (process_rule_1(c_in_cc, c_out_cc)) {
                rule1_timer->pause();
                std::cout << "Steiner point inserted by R1." << std::endl;
                continue;
            }
            rule1_timer->pause();

            rule2_timer->start();
            if (process_rule_2(c_in, c_in_cc)) {
                rule2_timer->pause();
                std::cout << "Steiner point inserted by R2." << std::endl;
                continue;
            }
            rule2_timer->pause();

            std::cout << "No Steiner point inserted. Marking c_in as OUTSIDE." << std::endl;
            c_in->info() = OUTSIDE;
            update_queue(c_in);
        }

        main_loop_timer->pause();

        exporter.export_svg("final_result.svg");

        total_timer->pause();
        
        // Print hierarchical timing report
        registry.print_all_hierarchies();
        std::cout << "Total iterations: " << iteration << std::endl;
    }


    void alpha_wrap_2::init() {

        // Insert bounding box points
        FT margin = 0.1 + offset_;
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
            Gate g;
            g.edge = *eit;
            g.priority = sq_minimal_delaunay_ball_radius(g.edge);
            queue_.push(g);
        }

    }

    bool alpha_wrap_2::is_gate(const Delaunay::Edge& e) const {

        // c_in and c_out are the faces
        auto c_in = e.first;
        int i = e.second;            
        auto c_out = c_in->neighbor(i);
                    
        return (c_in->info() != c_out->info());
    }

    // Return the squared radius of the minimal Delaunay ball through the edge
    FT alpha_wrap_2::sq_minimal_delaunay_ball_radius(const Delaunay::Edge& e) const {
        Delaunay::Face_handle c_1 = e.first;
        int i = e.second;            
        Delaunay::Face_handle c_2 = c_1->neighbor(i);

        // get edge vertices
        auto v1 = c_1->vertex(c_1->cw(i));
        auto v2 = c_1->vertex(c_1->ccw(i));
        auto p1 = v1->point();
        auto p2 = v2->point();

        auto mid = CGAL::midpoint(p1, p2);
        auto sq_rad = CGAL::squared_distance(p1, p2) / 4;

        // get non-edge vertices
        auto c_1_other = c_1->vertex(i)->point();
        auto c_2_other = c_2->vertex(c_2->cw(c_2->index(v1)))->point();

        // check if the other vertices are inside the circle

        if (c_1_other == p1 || c_1_other == p2) {
            std::ostringstream oss;
            oss << "Error: c_1_other is identical to one of the edge vertices: ";

            oss << "c_1 vertices: ";
            for (int vi = 0; vi < 3; ++vi) {
                oss << "(" << c_1->vertex(vi)->point() << ") ";
            }
            throw std::runtime_error(oss.str());
        }

        if (c_2_other == p1 || c_2_other == p2) {
            std::ostringstream oss;
            oss << "Error: c_2_other is identical to one of the edge vertices: ";

            oss << "c_2 vertices: ";
            for (int vi = 0; vi < 3; ++vi) {
                oss << "(" << c_2->vertex(vi)->point() << ") ";
            }

            throw std::runtime_error(oss.str());
        }

        // if Delaunay, use the smallest ball through both points
        if (CGAL::squared_distance(mid, c_1_other) > sq_rad &&
            CGAL::squared_distance(mid, c_2_other) > sq_rad) {
            return sq_rad;
        }

        // otherwise find the smaller Delaunay ball radius of the two triangles
        auto c1_sq_circum_rad = dt_.is_infinite(c_1) ? std::numeric_limits<FT>::infinity() : CGAL::squared_distance(dt_.circumcenter(c_1), p1);
        auto c2_sq_circum_rad = dt_.is_infinite(c_2) ? std::numeric_limits<FT>::infinity() : CGAL::squared_distance(dt_.circumcenter(c_2), p2);
        auto min_sq_circum_rad = std::min(c1_sq_circum_rad, c2_sq_circum_rad);
        
        if (min_sq_circum_rad == std::numeric_limits<FT>::infinity()) {
            throw std::runtime_error("Error: Both faces are infinite.");
        }
        return min_sq_circum_rad;
    }

    bool alpha_wrap_2::is_alpha_traversable(const Delaunay::Edge& e, const FT alpha) const {
        return sq_minimal_delaunay_ball_radius(e) >= alpha * alpha;
    }

    void alpha_wrap_2::update_queue(const Delaunay::Face_handle& fh){
        for(int i = 0; i < 3; ++i) {
            Delaunay::Edge e(fh, i);
            if (is_gate(e)) {
                Gate g;
                g.edge = e;
                g.priority = sq_minimal_delaunay_ball_radius(g.edge);
                queue_.push(g);
            }
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

        if (oracle_.do_intersect(c_in)) {
            // project circumcenter onto point set
            auto p_input = oracle_.closest_point(c_in_cc);

            // insert intersection with offset surface as steiner point
            if (oracle_.first_intersection(
                c_in_cc,
                p_input,
                steiner_point,
                offset_
            )) {
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
            if (is_gate(*eit)) {
                Gate g;
                g.edge = *eit;
                g.priority = sq_minimal_delaunay_ball_radius(g.edge);
                queue_.push(g);
            }
        }
    }
}


