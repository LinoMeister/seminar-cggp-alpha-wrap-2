// Public header for alpha_wrap_2
#ifndef AW2_ALPHA_WRAP_2_H
#define AW2_ALPHA_WRAP_2_H

#include "alpha_wrap_2/types.h"
#include "alpha_wrap_2/point_set_oracle_2.h"
#include "alpha_wrap_2/export_utils.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <iostream>
#include <queue>
#include <stack>
#include <filesystem>

namespace aw2 {
    using Oracle = point_set_oracle_2;


    struct AlgorithmConfig {
        // algorithm parameters
        FT alpha = 10.0;
        FT offset = 2.0;

        int max_iterations = 5000;

        // interval for exporting intermediate results
        int intermediate_steps = 50;

        // after this iteration we stop exporting intermediate results
        // this is useful when we only want to export the first few steps
        int export_step_limit = 1000; 
    };

    struct Gate {
        Delaunay::Edge edge;
        FT priority;

        std::pair<Point_2, Point_2> get_vertices() const;

        bool operator<(const Gate& other) const {
            return priority < other.priority;
        }
        
        bool operator>(const Gate& other) const {
            return priority > other.priority;
        }
    };
    
    
    class alpha_wrap_2 {
    public:
#ifdef USE_STACK_QUEUE
        using Queue = std::stack<Gate>;
#else
        using Queue = std::priority_queue<Gate, std::vector<Gate>, std::greater<Gate>>;
#endif

        alpha_wrap_2(const Oracle& oracle);

        void compute_wrap(AlgorithmConfig& config);

    private:

        void init();
        bool is_gate(const Delaunay::Edge& e) const;
        bool is_alpha_traversable(const Delaunay::Edge& e, const FT alpha) const;
        void update_queue(const Delaunay::Face_handle& fh);
        Point_2 infinite_face_cc(const Delaunay::Face_handle& c_in, const Delaunay::Face_handle& c_out, int edge_index) const;

        FT sq_minimal_delaunay_ball_radius(const Delaunay::Edge& e) const;
        bool process_rule_1(const Point_2& c_in_cc, const Point_2& c_out_cc);
        bool process_rule_2(const Delaunay::Face_handle& c_in, const Point_2& c_in_cc);
        void insert_steiner_point(const Point_2& steiner_point);


    public:

        const Oracle& oracle_;
        Delaunay dt_;

        Queue queue_;
        Gate candidate_gate_;

        FT alpha_;
        FT offset_;
    };
}

#endif // AW2_ALPHA_WRAP_2_H
