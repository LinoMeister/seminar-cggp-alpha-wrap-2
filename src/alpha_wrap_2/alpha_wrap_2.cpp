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

    void alpha_wrap_2::compute_wrap() {
        std::cout << "Computing alpha-wrap-2..." << std::endl;
        init();

        std::cout << "Queue contains " << queue_.size() << " gates." << std::endl;

        
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


