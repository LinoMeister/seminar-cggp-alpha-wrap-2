#include "alpha_wrap_2.h"


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

        std::cout << "Queue contains " << queue_.size() << " gates." << std::endl;

        int max_iterations = 5;
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
                std::cout << "c_out is infinite." << std::endl;
                const int inf_index = c_out->index(dt_.infinite_vertex());

                // construct a circumcenter for the infinite triangle
                Point_2 p1 = c_out->vertex((inf_index + 1) % 3)->point();
                Point_2 p2 = c_out->vertex((inf_index + 2) % 3)->point();
                Point_2 mid = CGAL::midpoint(p1, p2);
                
                K::Line_2 line(p1, p2);
                
                // check on which line the non-adjacent vertext of c_in lies
                Point_2 c_in_nonadjc = c_in->vertex(c_in->cw((i+1)%3))->point();
                std::cout << "Non-adjacent vertex of c_in: " << c_in_nonadjc << std::endl;
                auto side = line.oriented_side(c_in_nonadjc);

                int sign = (side == CGAL::ON_POSITIVE_SIDE) ? 1 : -1;

                auto dir = line.perpendicular(mid).direction().to_vector();
    
                Point_2 far_point = mid - sign * 10000 * dir;
                c_out_cc = CGAL::circumcenter(p1, p2, far_point);
            }
            else {
                c_out_cc = dt_.circumcenter(c_out);
            }

            std::cout << "Dual edge: " << c_out_cc << " -> " << c_in_cc << std::endl;

            Point_2 steiner_point;
            bool insert = oracle_.first_intersection(
                c_out_cc,
                c_in_cc,
                steiner_point,
                offset
            );

            if (insert) {
                std::cout << "R1 applies" << std::endl;
            }


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

            else {
                std::cout << "No Steiner point inserted. Marking c_in as OUTSIDE." << std::endl;
                c_in->info() = OUTSIDE;
                update_queue(c_in);
            }
            
        }


        
        export_svg("/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results/triangulation.svg");


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

    void alpha_wrap_2::export_svg(const std::string& filename,
                    double margin, double stroke_width,
                    double vertex_radius)
    {


        // First, compute bounding box of finite vertices
        double xmin = oracle_.bbox_.x_min;
        double ymin = oracle_.bbox_.y_min;
        double xmax = oracle_.bbox_.x_max;
        double ymax = oracle_.bbox_.y_max;


        if (xmin > xmax || ymin > ymax) {
            // No finite vertices
            return;
        }

        double width = xmax - xmin;
        double height = ymax - ymin;

        // Make an SVG canvas somewhat larger (margin)
        double svg_w = width + 2*margin;
        double svg_h = height + 2*margin;

        std::ofstream os(filename);
        os << "<?xml version=\"1.0\" standalone=\"no\"?>\n";
        os << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" )";
        os << "width=\"" << svg_w << "\" height=\"" << svg_h << "\">\n";

        // A helper to map a point to SVG coordinates
        auto to_svg = [&](const Point_2& p) {
            double x = (p.x() - xmin) + margin;
            // Flip Y so that larger y goes downward in SVG coordinate
            double y = (ymax - p.y()) + margin;
            return std::pair<double, double>(x, y);
        };

        // Draw edges of all finite faces
        os << R"(  <g stroke="black" stroke-width=")" << stroke_width
        << "\" fill=\"none\">\n";
        for (auto fit = dt_.finite_faces_begin(); fit != dt_.finite_faces_end(); ++fit) {
            Point_2 pa = fit->vertex(0)->point();
            Point_2 pb = fit->vertex(1)->point();
            Point_2 pc = fit->vertex(2)->point();
            auto sa = to_svg(pa);
            auto sb = to_svg(pb);
            auto sc = to_svg(pc);

            auto inside = (fit->info() == INSIDE);

            if (inside) {
                os << "    <polygon points=\""
                << std::fixed << std::setprecision(3)
                << sa.first << "," << sa.second << " "
                << sb.first << "," << sb.second << " "
                << sc.first << "," << sc.second
                << "\" fill=\"lightblue\" stroke=\"gray\" stroke-width=\"" << stroke_width/2 << "\" />\n";
            }
            else {
                os << "    <polygon points=\""
                << std::fixed << std::setprecision(3)
                << sa.first << "," << sa.second << " "
                << sb.first << "," << sb.second << " "
                << sc.first << "," << sc.second
                << "\" fill=\"none\" stroke=\"gray\" stroke-width=\"" << stroke_width/2 << "\" />\n";
            }

        }
        os << "  </g>\n";

        // Draw vertices as small circles
        os << "  <g stroke=\"red\" stroke-width=\""<< stroke_width <<"\" fill=\"red\">\n";
        for (auto vit = dt_.finite_vertices_begin(); vit != dt_.finite_vertices_end(); ++vit) {
            const Point_2& p = vit->point();
            auto sp = to_svg(p);
            os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
            << sp.first << "\" cy=\"" << sp.second
            << "\" r=\"" << vertex_radius << "\" />\n";
        }
        os << "  </g>\n";

        os << "  <g stroke=\"green\" stroke-width=\""<< stroke_width <<"\" fill=\"green\">\n";
        for (auto vit = oracle_.tree_.begin(); vit != oracle_.tree_.end(); ++vit) {
            const Point_2& p = *vit;
            auto sp = to_svg(p);
            os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
            << sp.first << "\" cy=\"" << sp.second
            << "\" r=\"" << vertex_radius << "\" />\n";
        }
        os << "  </g>\n";


        // Draw Voronoi diagram (dual of Delaunay triangulation)
        os << "  <g stroke=\"orange\" stroke-width=\"" << stroke_width/2 << "\" fill=\"none\">\n";
        for (auto eit = dt_.finite_edges_begin(); eit != dt_.finite_edges_end(); ++eit) {
            auto face = eit->first;
            int i = eit->second;
            auto neighbor = face->neighbor(i);
            // Only draw each Voronoi edge once
            if (dt_.is_infinite(face) || dt_.is_infinite(neighbor) || face > neighbor) continue;

            CGAL::Object o1 = dt_.dual(eit);
            if (const Segment_2* s = CGAL::object_cast<Segment_2>(&o1)) {

                Point_2 p(s->source().x(), s->source().y());
                Point_2 q(s->target().x(), s->target().y());
                Point_2 o;
                auto intersects = oracle_.first_intersection(p, q, o, 5, 1.0);

                auto sa = to_svg(s->source());
                auto sb = to_svg(s->target());

                if (intersects) {
                    os << "    <line stroke=\"red\" x1=\"" << sa.first << "\" y1=\"" << sa.second
                    << "\" x2=\"" << sb.first << "\" y2=\"" << sb.second << "\" />\n";
                    auto so = to_svg(o);
                    os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
                    << so.first << "\" cy=\"" << so.second
                    << "\" r=\"" << vertex_radius*2 << "\" fill=\"purple\" />\n";
                }
                else {
                    os << "    <line stroke=\"orange\" x1=\"" << sa.first << "\" y1=\"" << sa.second
                    << "\" x2=\"" << sb.first << "\" y2=\"" << sb.second << "\" />\n";
                }



            }
        }
        os << "  </g>\n";

        // draw candidate edge
        os << "  <g stroke=\"green\" stroke-width=\"" << stroke_width/2 << "\" fill=\"none\">\n";
        auto v1 = candidate_gate_.edge.first->vertex(candidate_gate_.edge.first->cw(candidate_gate_.edge.second))->point();
        auto v2 = candidate_gate_.edge.first->vertex(candidate_gate_.edge.first->ccw(candidate_gate_.edge.second))->point();
        auto sv1 = to_svg(v1);
        auto sv2 = to_svg(v2);
        os << "    <line stroke=\"green\" x1=\"" << sv1.first << "\" y1=\"" << sv1.second
        << "\" x2=\"" << sv2.first << "\" y2=\"" << sv2.second << "\" />\n";
        os << "  </g>\n";


        os << "</svg>\n";
        os.close();
    }

}


