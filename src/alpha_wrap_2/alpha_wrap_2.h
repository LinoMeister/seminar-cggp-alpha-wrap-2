#include "types.h"
#include "export_utils.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <iostream>


namespace aw2 {
    using Oracle = point_set_oracle_2;


    void aw2_test(const Oracle& oracle);


    struct Gate {
        Delaunay::Edge edge;
        FT priority;
    };
    
    
    class alpha_wrap_2 {

        using Queue = std::stack<Gate>;

    public:
        alpha_wrap_2(const Oracle& oracle);

        void compute_wrap();

    private:

        void init();
        bool is_gate(const Delaunay::Edge& e) const;

        bool is_alpha_traversable(const Delaunay::Edge& e, const FT alpha) const;

    public:

        const Oracle& oracle_;
        Delaunay dt_;

        Queue queue_;
        Gate candidate_gate_;
    };
}