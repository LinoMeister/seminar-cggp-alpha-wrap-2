#include "alpha_wrap_2.h"


namespace aw2 {

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
        export_svg(dt, oracle, "/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results/triangulation.svg");
    }

    alpha_wrap_2::alpha_wrap_2(const Oracle& oracle) : oracle_(oracle) {
        

    }

    void alpha_wrap_2::compute_wrap(FT alpha, FT offset) {
        std::cout << "Computing alpha-wrap-2..." << std::endl;
        init();

        std::cout << "Queue contains " << queue_.size() << " gates." << std::endl;

        int max_iterations = 1000;
        int iteration = 0;

        while (!queue_.empty()) {
            std::cout << "Iteration: " << iteration << " Queue size: " << queue_.size() << std::endl;

            if (iteration++ > max_iterations) {
                std::cout << "Reached maximum number of iterations (" << max_iterations << "). Stopping." << std::endl;
                break;
            }

            candidate_gate_ = queue_.top();
            queue_.pop();

            if (!is_gate(candidate_gate_.edge)) {
                continue; // Not a gate anymore
            }

            if (!is_alpha_traversable(candidate_gate_.edge, alpha)) {
                continue; // Not traversable
            }

            std::cout << "Passed checks." << std::endl;

            // Traverse the gate
            auto c_1 = candidate_gate_.edge.first;
            int i = candidate_gate_.edge.second;            
            auto c_2 = c_1->neighbor(i);
            
            // Find c_in and c_out
            Delaunay::Face_handle c_in = c_1;
            Delaunay::Face_handle c_out = c_2;

            if (c_in->info() == OUTSIDE && c_in->info() == INSIDE) {
                c_in = c_2;
                c_out = c_1;
            } 
            CGAL::Object o1 = dt_.dual(candidate_gate_.edge);
            auto dual_edge = CGAL::object_cast<Segment_2>(&o1);


            if (dual_edge == nullptr) {
                std::cout << "Dual edge is not a segment." << std::endl;
                continue;
            }


            std::cout << "Checking R1" << std::endl;

            Point_2 steiner_point;
            bool insert = oracle_.first_intersection(
                dual_edge->source(),
                dual_edge->target(),
                steiner_point,
                offset
            );


            if (!insert) {
                std::cout << "Checking R2" << std::endl;
                insert = oracle_.do_intersect(c_in);
                if (insert) {
                    auto circum_center = dt_.circumcenter(c_in);

                    // TODO: for now just use closest point
                    steiner_point = oracle_.closest_point(circum_center);
                }
            }

            if (insert) {
                std::cout << "Inserting Steiner point at " << steiner_point << std::endl;
                // find gates in queue that will be destroyed after insertion
                std::stack<Gate> temp_queue;
                while (!queue_.empty()) {
                    auto g = queue_.top();
                    queue_.pop();   
                    if (g.edge.first == c_in || g.edge.first == c_out ||
                        g.edge.first->neighbor(g.edge.second) == c_in ||
                        g.edge.first->neighbor(g.edge.second) == c_out) {
                        continue; // gate will be destroyed
                    }
                    temp_queue.push(g);
                }
                queue_ = temp_queue;

                // insert Steiner point
                auto vh = dt_.insert(steiner_point);

                // Update face labels
                for (auto fit = dt_.incident_faces(vh); ; ++fit) {
                    if (dt_.is_infinite(fit)) {
                        fit->info() = OUTSIDE;
                    } else {
                        fit->info() = INSIDE;
                    }

                    if (++fit == dt_.incident_faces(vh)) break;
                }

                // Add new gates to the queue
                for (auto eit = dt_.incident_edges(vh); ; ++eit) {
                    if (is_gate(*eit)) {
                        Gate g;
                        g.edge = *eit;
                        g.priority = 0.0; // TODO: Compute priority
                        queue_.push(g);
                    }

                    if (++eit == dt_.incident_edges(vh)) break;
                }
            }

            
        }


        
        export_svg(dt_, oracle_, "/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results/triangulation.svg");


    }


    void alpha_wrap_2::init() {

        // Insert bounding box points
        auto bbox = oracle_.bbox_;
        std::vector<Point_2> pts_bbox = {
            {bbox.x_min,bbox.y_min}, {bbox.x_min,bbox.y_max}, {bbox.x_max,bbox.y_min}, {bbox.x_max,bbox.y_max}
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

}


