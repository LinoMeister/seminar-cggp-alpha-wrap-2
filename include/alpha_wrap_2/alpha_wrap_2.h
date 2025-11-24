// Public header for alpha_wrap_2
#ifndef AW2_ALPHA_WRAP_2_H
#define AW2_ALPHA_WRAP_2_H

#include "alpha_wrap_2/types.h"
#include "alpha_wrap_2/point_set_oracle_2.h"
#include "alpha_wrap_2/export_utils.h"
#include "alpha_wrap_2/statistics.h"
#include "alpha_wrap_2/traversability.h"
#include "alpha_wrap_2/timer.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_circular_kernel_2.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <iostream>
#include <queue>
#include <stack>
#include <filesystem>

namespace aw2 {

    // Forward declaration
    class Timer;

    // type definitions
    using Oracle = point_set_oracle_2;

    #ifdef USE_STACK_QUEUE
        using Queue = std::stack<Gate>;
    #else
        using Queue = std::priority_queue<Gate, std::vector<Gate>, std::less<Gate>>;
    #endif

    enum TraversabilityMethod {
        CONSTANT_ALPHA,
        DEVIATION_BASED,
        INTERSECTION_BASED
    };

    struct AlgorithmConfig {
        // algorithm parameters
        FT alpha = 10.0;
        FT offset = 2.0;

        TraversabilityMethod traversability_method = CONSTANT_ALPHA;

        // Method-specific parameters
        TraversabilityParams traversability_params;

        int max_iterations = 5000;

        // interval for exporting intermediate results
        int intermediate_steps = 50;

        // after this iteration we stop exporting intermediate results
        // this is useful when we only want to export the first few steps
        int export_step_limit = 1000; 

        std::string output_directory = "/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results/";

        // visualization style (default, clean, outside_filled)
        std::string style = "default";
    };



    struct EdgeAdjacencyInfo {
        Delaunay::Edge edge;
        Point_2 cc_inside;
        Point_2 cc_outside;
        bool outside_infinite;
    };


    class alpha_wrap_2 {
    public:

        // algorithm state
        const Oracle& oracle_;
        Delaunay dt_;

        Queue queue_;
        Gate candidate_gate_;

        // algorithm configuration
        FT alpha_;
        FT offset_;
        AlgorithmConfig config_;

        Traversability* traversability_;

        int max_iterations_;

        FT bbox_diagonal_length_;
        Point_2 dt_bbox_min_;
        Point_2 dt_bbox_max_;

        std::vector<Segment_2> wrap_edges_;

        // statistics tracking
        AlgorithmStatistics statistics_;

        // timers for performance tracking
        TimerRegistry& registry_ = TimerRegistry::instance();
        Timer* total_timer_ = nullptr;
        Timer* init_timer_ = nullptr;
        Timer* main_loop_timer_ = nullptr;
        Timer* rule1_timer_ = nullptr;
        Timer* rule2_timer_ = nullptr;
        Timer* gate_processing_timer_ = nullptr;
        Timer* extraction_timer_ = nullptr;

        // initialization and running
        alpha_wrap_2(const Oracle& oracle);
        ~alpha_wrap_2();
        void init(AlgorithmConfig& config);
        void run();
        
        // Get statistics (can be called after compute_wrap)
        const AlgorithmStatistics& get_statistics() const { return statistics_; }
        EdgeAdjacencyInfo gate_adjacency_info(const Delaunay::Edge& edge) const;


    private:

        // gate and traversability processing methods
        bool is_gate(const Delaunay::Edge& e) const;
        FT sq_minimal_delaunay_ball_radius(const Gate& gate) const;

        // rule processing
        bool process_rule_1(const Point_2& c_in_cc, const Point_2& c_out_cc);
        bool process_rule_2(const Delaunay::Face_handle& c_in, const Point_2& c_in_cc);

        // update
        void insert_steiner_point(const Point_2& steiner_point);
        void add_gate_to_queue(const Delaunay::Edge& edge);
        void update_queue(const Delaunay::Face_handle& fh);

        // utils
        Point_2 infinite_face_cc(const Delaunay::Face_handle& c_in, const Delaunay::Face_handle& c_out, int edge_index) const;
        void extract_wrap_surface();
    };
}

#endif // AW2_ALPHA_WRAP_2_H
