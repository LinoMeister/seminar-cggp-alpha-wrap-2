#include <alpha_wrap_2/alpha_wrap_2.h>


namespace aw2 {

    std::pair<Point_2, Point_2> Gate::get_vertices() const {
        auto v1 = edge.first->vertex(edge.first->cw(edge.second))->point();
        auto v2 = edge.first->vertex(edge.first->ccw(edge.second))->point();
        return std::make_pair(v1, v2);
    }

    void aw2_test(const Oracle& oracle) {
        std::vector<Point_2> pts_test = {
            {0,0}, {700,0}, {0,500}, {250,800}, {670, 200}, {400, 700}
        };

        auto bbox = oracle.bbox_;
        std::vector<Point_2> pts_bbox = {
            {bbox.x_min,bbox.y_min}, {bbox.x_min,bbox.y_max}, {bbox.x_max,bbox.y_min}, {bbox.x_max,bbox.y_max}
        };

        Delaunay dt;
        dt.insert(pts_bbox.begin(), pts_bbox.end());

        Points pts = {
            {100,100}, {200,200}, {300,300}, {400,400}, {500,500},
            {600,100}, {100,600}, {300,500}, {500,300},
            {150,400}, {400,150}, {250,250}, {350,350}
        };
        dt.insert(pts.begin(), pts.end());

        for (auto fit = dt.finite_faces_begin(); fit != dt.finite_faces_end(); ++fit) {
            auto intersects = oracle.do_intersect(fit);
            if (intersects) {
                fit->info() = INSIDE;
            } else {
                fit->info() = OUTSIDE;  
            }
        }

        std::cout << "exporting svg image" << std::endl;
        //export_svg(dt, oracle, "/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results/triangulation.svg");
    }

    alpha_wrap_2::alpha_wrap_2(const Oracle& oracle) : oracle_(oracle) {
        

    }

    void alpha_wrap_2::compute_wrap(FT alpha, FT offset) {
        std::cout << "Computing alpha-wrap-2 with " << "alpha: " << alpha << " offset: " << offset << std::endl;
        alpha_ = alpha;
        offset_ = offset;
        init();

        auto style = StyleConfig{};
        style.voronoi_diagram = {"pink", 0.6};
        style.use_gradients = true;
        style.use_opacity = true;
        style.opacity = 1.0;
        style.scheme = ColorScheme::GRADIENT;
        alpha_wrap_2_exporter exporter(*this, style);

        std::cout << "Queue contains " << queue_.size() << " gates." << std::endl;

        int max_iterations = 2000;
        int iteration = 0;

        while (!queue_.empty()) {
            std::cout << "\nIteration: " << iteration << " Queue size: " << queue_.size() << std::endl;

            if (iteration++ > max_iterations) {
                std::cout << "Reached maximum number of iterations (" << max_iterations << "). Stopping." << std::endl;
                break;
            }

            candidate_gate_ = queue_.top();
            queue_.pop();

            std::cout << "Candidate gate: " << candidate_gate_.get_vertices().first << " -- " << candidate_gate_.get_vertices().second << std::endl;

            if (!is_gate(candidate_gate_.edge)) {
                continue; // Not a gate anymore
            }

            if (!is_alpha_traversable(candidate_gate_.edge, alpha)) {
                continue; // Not traversable
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
                std::cout << "Error: Both faces have the same label." << std::endl;
                continue;
            }

            if (dt_.is_infinite(c_in)) {
                std::cout << "Error: c_in is infinite." << std::endl;
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

            std::cout << "Dual edge: " << c_out_cc << " -> " << c_in_cc << std::endl;

            if (process_rule_1(c_in_cc, c_out_cc)) {
                std::cout << "Steiner point inserted by R1." << std::endl;
                continue;
            }

            if (process_rule_2(c_in, c_in_cc)) {
                std::cout << "Steiner point inserted by R2." << std::endl;
                continue;
            }

            std::cout << "No Steiner point inserted. Marking c_in as OUTSIDE." << std::endl;
            c_in->info() = OUTSIDE;
            update_queue(c_in);    
        }


        
        exporter.export_svg("/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results/triangulation.svg");
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

                Gate g;
                g.edge = *eit;
                g.priority = 0.0; // TODO: Compute priority
                queue_.push(g);
                continue;
            }

            if (dt_.is_infinite(c_2) && !dt_.is_infinite(c_1)) {
                c_2->info() = OUTSIDE;
                c_1->info() = INSIDE;

                Gate g;
                g.edge = *eit;
                g.priority = 0.0; // TODO: Compute priority
                queue_.push(g);
            }
        }

    }

    bool alpha_wrap_2::is_gate(const Delaunay::Edge& e) const {

        // c_in and c_out are the faces
        auto c_in = e.first;
        int i = e.second;            
        auto c_out = c_in->neighbor(i);
                    
        return (c_in->info() != c_out->info());
    }

    bool alpha_wrap_2::is_alpha_traversable(const Delaunay::Edge& e, const FT alpha) const {
        // c_in and c_out are the faces
        auto c_in = e.first;
        int i = e.second;            
        auto c_out = c_in->neighbor(i);

        
        // TODO: Correct implementation
        // For now use the edge length
        auto v1 = c_in->vertex(c_in->cw(i))->point();
        auto v2 = c_in->vertex(c_in->ccw(i))->point();
        return CGAL::squared_distance(v1, v2) >= alpha * alpha;
    }

    void alpha_wrap_2::update_queue(const Delaunay::Face_handle& fh){
        for(int i = 0; i < 3; ++i) {
            Delaunay::Edge e(fh, i);
            if (is_gate(e)) {
                Gate g;
                g.edge = e;
                g.priority = 0.0; // TODO: Compute priority
                queue_.push(g);
            }
        }
    }

    Point_2 alpha_wrap_2::infinite_face_cc(const Delaunay::Face_handle& c_in, const Delaunay::Face_handle& c_out, int edge_index) {
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
                g.priority = 0.0; // TODO: Compute priority
                queue_.push(g);
            }
        }
    }
}


