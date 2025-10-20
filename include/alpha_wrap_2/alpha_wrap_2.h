// Public header for alpha_wrap_2
#ifndef AW2_ALPHA_WRAP_2_H
#define AW2_ALPHA_WRAP_2_H

#include "alpha_wrap_2/types.h"
#include "alpha_wrap_2/point_set_oracle_2.h"
#include "alpha_wrap_2/export_utils.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_circular_kernel_2.h>
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

    struct EdgeAdjacencyInfo {
        Delaunay::Edge edge;
        Face_handle inside;
        Face_handle outside;
        Point_2 cc_inside;
        Point_2 cc_outside;
        bool outside_infinite;

        std::pair<Point_2, Point_2> get_points() const;
    };
    
    
    class alpha_wrap_2 {
    public:
#ifdef USE_STACK_QUEUE
        using Queue = std::stack<Gate>;
#else
        using Queue = std::priority_queue<Gate, std::vector<Gate>, std::less<Gate>>;
#endif
        void init(AlgorithmConfig& config);
        alpha_wrap_2(const Oracle& oracle);

        void compute_wrap(AlgorithmConfig& config);
        FT adaptive_alpha(const Segment_2& seg) const;


    private:

        bool is_gate(const Delaunay::Edge& e) const;
        bool is_alpha_traversable(const Gate& g) const;
        void update_queue(const Delaunay::Face_handle& fh);
        Point_2 infinite_face_cc(const Delaunay::Face_handle& c_in, const Delaunay::Face_handle& c_out, int edge_index) const;

        FT sq_minimal_delaunay_ball_radius(const Delaunay::Edge& e) const;
        bool process_rule_1(const Point_2& c_in_cc, const Point_2& c_out_cc);
        bool process_rule_2(const Delaunay::Face_handle& c_in, const Point_2& c_in_cc);
        void insert_steiner_point(const Point_2& steiner_point);
        EdgeAdjacencyInfo gate_adjacency_info(const Delaunay::Edge& edge) const;
        void extract_wrap_surface();

        FT subsegment_deviation(const Segment_2& seg) const;
        FT segment_deviation(const Segment_2& seg) const;


    public:

        const Oracle& oracle_;
        Delaunay dt_;

        Queue queue_;
        Gate candidate_gate_;

        FT alpha_;
        FT alpha_min_;
        FT alpha_max_;
        FT offset_;

        int max_iterations_;

        FT bbox_diagonal_length_;

        std::vector<Segment_2> wrap_edges_;
    };
}

#endif // AW2_ALPHA_WRAP_2_H
