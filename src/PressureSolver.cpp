#include "PressureSolver.hpp"

#include <cmath>
#include <iostream>

double Jacobi::solve(Fields &field, Grid &grid, const std::vector<std::unique_ptr<Boundary>> &boundaries) {
    double dx = grid.dx();
    double dy = grid.dy();

    double coeff = 1 / (2.0 * (1.0 / (dx * dx) + 1.0 / (dy * dy)));
    int i, j;

    auto p_old = field.p_matrix();
    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            field.p(i, j) = coeff * (Discretization::sor_helper(p_old, i, j) - field.rs(i, j));
        }
    }

    double res = 0.0;
    double rloc = 0.0;

    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            double val = Discretization::laplacian(field.p_matrix(), i, j) - field.rs(i, j);
            rloc += (val * val);
        }
    }

    return rloc;
}

SOR::SOR(double omega) : _omega(omega) {}

double SOR::solve(Fields &field, Grid &grid, const std::vector<std::unique_ptr<Boundary>> &boundaries) {

    double dx = grid.dx();
    double dy = grid.dy();

    double coeff = _omega / (2.0 * (1.0 / (dx * dx) + 1.0 / (dy * dy))); // = _omega * h^2 / 4.0, if dx == dy == h

    int i, j;

    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            field.p(i, j) = (1.0 - _omega) * field.p(i, j) +
                            coeff * (Discretization::sor_helper(field.p_matrix(), i, j) - field.rs(i, j));
        }
    }

    double res = 0.0;
    double rloc = 0.0;

    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            double val = Discretization::laplacian(field.p_matrix(), i, j) - field.rs(i, j);
            rloc += (val * val);
        }
    }

    return rloc;
}

WeightedJacobi::WeightedJacobi(double omega) : _omega(omega) {}

double WeightedJacobi::solve(Fields &field, Grid &grid, const std::vector<std::unique_ptr<Boundary>> &boundaries) {

    double dx = grid.dx();
    double dy = grid.dy();

    double coeff = _omega / (2.0 * (1.0 / (dx * dx) + 1.0 / (dy * dy))); // = _omega * h^2 / 4.0, if dx == dy == h

    int i, j;
    auto p_old = field.p_matrix();
    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            field.p(i, j) =
                (1.0 - _omega) * p_old(i, j) + coeff * (Discretization::sor_helper(p_old, i, j) - field.rs(i, j));
        }
    }

    double res = 0.0;
    double rloc = 0.0;

    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            double val = Discretization::laplacian(field.p_matrix(), i, j) - field.rs(i, j);
            rloc += (val * val);
        }
    }

    return rloc;
}

double GaussSeidel::solve(Fields &field, Grid &grid, const std::vector<std::unique_ptr<Boundary>> &boundaries) {
    double dx = grid.dx();
    double dy = grid.dy();

    double coeff = 1.0 / (2.0 * (1.0 / (dx * dx) + 1.0 / (dy * dy))); // = h^2 / 4.0, if dx == dy == h

    int i, j;

    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            field.p(i, j) = coeff * (Discretization::sor_helper(field.p_matrix(), i, j) - field.rs(i, j));
        }
    }

    double res = 0.0;
    double rloc = 0.0;

    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            double val = Discretization::laplacian(field.p_matrix(), i, j) - field.rs(i, j);
            rloc += (val * val);
        }
    }

    return rloc;
}

Richardson::Richardson(double omega) : _omega(omega) {
    if (omega == 0) {
        std::cout << "Invalid Omega for this scheme chosen. Setting omega to be 1.0" << std::endl;
        _omega = 1.0;
    }
}

double Richardson::solve(Fields &field, Grid &grid, const std::vector<std::unique_ptr<Boundary>> &boundaries) {
    double dx = grid.dx();
    double dy = grid.dy();

    int i, j;

    auto p_old = field.p_matrix();

    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            field.p(i, j) = p_old(i, j) + _omega * (field.rs(i, j) - Discretization::laplacian(field.p_matrix(), i, j));
        }
    }

    double res = 0.0;
    double rloc = 0.0;

    for (auto currentCell : grid.fluid_cells()) {
        i = currentCell->i();
        j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            double val = Discretization::laplacian(field.p_matrix(), i, j) - field.rs(i, j);
            rloc += (val * val);
        }
    }

    return rloc;
}

GradientMethods::GradientMethods(Fields &field) {
    int imax = field.p_matrix().imax();
    int jmax = field.p_matrix().jmax();

    d = Matrix<double>(imax, jmax, 0.0);
    residual = Matrix<double>(imax, jmax, 0.0);
}

ConjugateGradient::ConjugateGradient(Fields &field) : GradientMethods(field) {}

double ConjugateGradient::solve(Fields &field, Grid &grid, const std::vector<std::unique_ptr<Boundary>> &boundaries) {

    auto pressure = field.p_matrix();
    auto rhs = field.rs_matrix();

    int imax, jmax;

    imax = field.p_matrix().imax();
    jmax = field.p_matrix().jmax();

    for (auto currentCell : grid.fluid_cells()) {
        int i = currentCell->i();
        int j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            residual(i, j) = rhs(i, j) - Discretization::laplacian(pressure, i, j);
        }
    }

    if (iter == 0) {
        d = residual;
    }

    Matrix<double> q(imax, jmax, 0.0);

    for (auto currentCell : grid.fluid_cells()) {
        int i = currentCell->i();
        int j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            q(i, j) =
                Discretization::laplacian(d, i, j); // the q = Ad for this system would be laplacian of the d vector
        }
    }

    double alpha_num = 0;
    double alpha_den = 0;
    double alpha = 0;

    for (auto currentCell : grid.fluid_cells()) {
        int i = currentCell->i();
        int j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            alpha_num += residual(i, j) * residual(i, j); // num of alpha is delta_new = r^T * r
            alpha_den += d(i, j) * q(i, j);               // den of alpha is d^T * q
        }
    }

    alpha = alpha_num / alpha_den;

    double beta_den = 0;
    double beta_num = 0;
    double beta = 0;

    for (auto currentCell : grid.fluid_cells()) {
        int i = currentCell->i();
        int j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {

            field.p(i, j) = pressure(i, j) + alpha * d(i, j);
            beta_den += residual(i, j) * residual(i, j);
            if (iter == 50) {
                iter = 0;
                residual(i, j) = rhs(i, j) - Discretization::laplacian(pressure, i, j);
            } else {
                residual(i, j) -= alpha * q(i, j);
            }
            beta_num += residual(i, j) * residual(i, j);
        }
    }

    beta = beta_num / beta_den;

    for (auto currentCell : grid.fluid_cells()) {
        int i = currentCell->i();
        int j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            d(i, j) = residual(i, j) + beta * d(i, j);
        }
    }

    iter++;

    double res = 0.0;
    double rloc = 0.0;

    for (auto currentCell : grid.fluid_cells()) {
        int i = currentCell->i();
        int j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            double val = Discretization::laplacian(field.p_matrix(), i, j) - field.rs(i, j);
            rloc += (val * val);
        }
    }

    return rloc;
}

MultiGrid::MultiGrid(int user_levels, int iter1, int iter2, std::vector<Grid> multigrid_grid,
                     std::vector<Fields> multigrid_field)
    : _max_multi_grid_level(user_levels), _smoothing_pre_recur(iter1), _smoothing_post_recur(iter2),
      _multigrid_grid(multigrid_grid), _multigrid_field(multigrid_field) {
    create_boundaries();
}

MultiGridVCycle::MultiGridVCycle(int user_levels, int iter1, int iter2, std::vector<Grid> multigrid_grid,
                                 std::vector<Fields> multigrid_field)
    : MultiGrid(user_levels, iter1, iter2, multigrid_grid, multigrid_field) {}

void MultiGrid::create_boundaries() {
    for (int k = 0; k < _max_multi_grid_level; k++) {
        // Construct boundaries
        std::vector<std::unique_ptr<Boundary>> creating_boundaries;

        if (not _multigrid_grid[k].fixed_wall_cells().empty()) {
            creating_boundaries.push_back(std::make_unique<FixedWallBoundary>(_multigrid_grid[k].fixed_wall_cells()));
        }

        if (not _multigrid_grid[k].moving_wall_cells().empty()) {
            creating_boundaries.push_back(std::make_unique<MovingWallBoundary>(_multigrid_grid[k].moving_wall_cells(),
                                                                               LidDrivenCavity::wall_velocity));
        }

        if (not _multigrid_grid[k].hot_fixed_wall_cells().empty()) {
            creating_boundaries.push_back(
                std::make_unique<FixedWallBoundary>(_multigrid_grid[k].hot_fixed_wall_cells(), 0.0));
        }

        if (not _multigrid_grid[k].cold_fixed_wall_cells().empty()) {
            creating_boundaries.push_back(
                std::make_unique<FixedWallBoundary>(_multigrid_grid[k].cold_fixed_wall_cells(), 0.0));
        }

        if (not _multigrid_grid[k].adiabatic_fixed_wall_cells().empty()) {
            creating_boundaries.push_back(
                std::make_unique<AdiabaticWallBoundary>(_multigrid_grid[k].adiabatic_fixed_wall_cells()));
        }

        if (not _multigrid_grid[k].inflow_cells().empty()) {
            creating_boundaries.push_back(std::make_unique<InFlow>(
                _multigrid_grid[k].inflow_cells(), std::map<int, double>{{PlaneShearFlow::inflow_wall_id, 1.0}}));
        }

        if (not _multigrid_grid[k].outflow_cells().empty()) {
            creating_boundaries.push_back(std::make_unique<OutFlow>(_multigrid_grid[k].outflow_cells(), 0.0));
        }
        // std::cout << creating_boundaries.size() << std::endl;
        _multigrid_boundaries.push_back(std::move(creating_boundaries));
    }
}

double MultiGridVCycle::solve(Fields &field, Grid &grid, const std::vector<std::unique_ptr<Boundary>> &boundaries) {

    double dx = grid.dx();
    double dy = grid.dy();

    int imax = grid.imax(); // accessing only fluid cells
    int jmax = grid.jmax(); // accessing only fluid cells

    auto p = field.p_matrix();
    auto rs = field.rs_matrix();

    field.p_matrix() = recursiveMultiGridCycle(field, p, rs, 0, dx, dy);

    double rloc = 0.0;
    for (auto currentCell : grid.fluid_cells()) {
        int i = currentCell->i();
        int j = currentCell->j();
        if (i != 0 && j != 0 && i != grid.domain().size_x + 1 && j != grid.domain().size_y + 1) {
            double val = Discretization::laplacian(field.p_matrix(), i, j) - field.rs(i, j);
            rloc += (val * val);
        }
    }

    return rloc;
};

Matrix<double> MultiGridVCycle::recursiveMultiGridCycle(Fields &field, Matrix<double> p, Matrix<double> rs,
                                                        int current_level, double dx, double dy) {
    if (current_level == _max_multi_grid_level - 1) {

        p = smoother(p, rs, current_level, 5*(_smoothing_pre_recur + _smoothing_post_recur), dx, dy, _multigrid_boundaries[current_level]);
        
        return p;

    } else {

        p = smoother(p, rs, current_level, _smoothing_pre_recur, dx, dy, _multigrid_boundaries[current_level]);

        Matrix<double> residual_ = residual(p, rs, dx, dy);

        auto coarse_residual = restrictor(residual_);

        auto error = Matrix<double>(coarse_residual.imax(), coarse_residual.jmax(), 0.0);

        error = recursiveMultiGridCycle(_multigrid_field[current_level+1], error, coarse_residual, current_level + 1, 2 * dx, 2 * dy);

        auto error_fine = prolongator(error);

        for (int i = 0; i < error_fine.imax(); ++i) {
            for (int j = 0; j < error_fine.jmax(); ++j) {
                p(i, j) = p(i, j) + error_fine(i, j);
                //                 if(std::isnan(p(i,j)))
                // {
                //     // std::cout << coeff << std::endl;
                //     // std::cout << sor_helper << std::endl;
                //     // std::cout << rs(i,j) << std::endl;
                //     std::cout << "Here" << std::endl;
                //     // std::cout << it << std::endl;
                //     exit(0);
                // }
            }
        }

        p = smoother(p, rs, current_level, _smoothing_post_recur, dx, dy, _multigrid_boundaries[current_level]);

        return p;
    }
}

Matrix<double> MultiGrid::residual(Matrix<double> p, Matrix<double> rs, double dx, double dy) {
    int imax = p.imax() - 2;
    int jmax = p.jmax() - 2;

    auto residuals = Matrix<double>(imax + 2, jmax + 2, 0.0);

    for (int i = 1; i <= imax; i++) {
        for (int j = 1; j <= jmax; j++) {
            auto helper = (p(i + 1, j) - 2.0 * p(i, j) + p(i - 1, j)) / (dx * dx) +
                          (p(i, j + 1) - 2.0 * p(i, j) + p(i, j - 1)) / (dy * dy);
            residuals(i, j) = rs(i, j) - helper;
        }
    }

    return residuals;
}

Matrix<double> MultiGrid::smoother(Matrix<double> error, Matrix<double> rs, int level, int iter, double dx, double dy,
                                   const std::vector<std::unique_ptr<Boundary>> &multigrid_boundary) {
    int imax = error.imax() - 2;
    int jmax = error.jmax() - 2;

    double coeff = 1 / (2.0 * (1.0 / (dx * dx) + 1.0 / (dy * dy)));

    auto error_new = error;
    for (int it = 0; it < iter; ++it) {
        for (int j = 1; j <= jmax; ++j) {
            for (int i = 1; i <= imax; ++i) {
                auto sor_helper =
                    (error(i + 1, j) + error(i - 1, j)) / (dx * dx) + (error(i, j + 1) + error(i, j - 1)) / (dy * dy);
                error_new(i, j) = coeff * (sor_helper - rs(i, j));
                
            }
        }

        // Applying boundary conditions
        _multigrid_field[level].p_matrix() = error_new;
        for (const auto &boundary : _multigrid_boundaries[level]) {
            boundary->apply_pressures(_multigrid_field[level]);
        }

        // error_new = _multigrid_field[level].p_matrix();
        // std::cout << error_new(imax+1, jmax/2) << " ";


        // for (int i = 1; i <= imax; i++) {
        //     error_new(i, 0) = error_new(i, 1);
        //     error_new(i, jmax + 1) = error_new(i, jmax);
        // }

        // for (int j = 1; j <= jmax; j++) {
        //     error_new(0, j) = error_new(1, j);
        //     error_new(imax + 1, j) = -error_new(imax, j);
        // }

        error = error_new;
    }

    // Karthik do some work 

    return error_new;
}

Matrix<double> MultiGrid::restrictor(Matrix<double> fine) {
    int imax = fine.imax() / 2 - 1;
    int jmax = fine.jmax() / 2 - 1;

    Matrix<double> coarse = Matrix<double>(imax + 2, jmax + 2, 0.0);

    // Slide 57 from https://www.math.hkust.edu.hk/~mawang/teaching/math532/mgtut.pdf
    for (int i = 1; i <= imax; ++i) {
        for (int j = 1; j <= jmax; ++j) {
            coarse(i, j) = 0.25 * fine(2 * i, 2 * j) +
                           0.125 * (fine(2 * i - 1, 2 * j) + fine(2 * i + i, 2 * j) + fine(2 * i, 2 * j - 1) +
                                    fine(2 * i, 2 * j + 1)) +
                           0.0625 * (fine(2 * i - 1, 2 * j - 1) + fine(2 * i - 1, 2 * j + 1) +
                                     fine(2 * i + 1, 2 * j - 1) + fine(2 * i + 1, 2 * j + 1));
        }
    }

    for (int i = 1; i <= imax; ++i) {
        coarse(i, 0) = fine(2 * i, 0) + 0.5 * (fine(2 * i - 1, 0) + fine(2 * i + 1, 0));
        coarse(i, jmax + 1) =
            fine(2 * i, 2 * jmax + 1) + 0.5 * (fine(2 * i - 1, 2 * jmax + 1) + fine(2 * i + 1, 2 * jmax + 1));
    }

    for (int j = 1; j <= jmax; ++j) {
        coarse(0, j) = fine(0, 2 * j) + 0.5 * (fine(0, 2 * j - 1) + fine(0, 2 * j + 1));
        coarse(imax + 1, j) =
            fine(2 * imax + 1, 2 * j) + 0.5 * (fine(2 * imax + 1, 2 * j - 1) + fine(2 * imax + 1, 2 * j + 1));
    }

    return coarse;
}

Matrix<double> MultiGrid::prolongator(Matrix<double> coarse) {
    int imax = coarse.imax() - 2;
    int jmax = coarse.jmax() - 2;

    Matrix<double> fine = Matrix<double>(2 * imax + 2, 2 * jmax + 2, 0.0);

    // Slide 56 from https://www.math.hkust.edu.hk/~mawang/teaching/math532/mgtut.pdf

    for (int i = 0; i <= imax; i++) {
        for (int j = 0; j <= jmax; j++) {
            fine(2 * i, 2 * j) = coarse(i, j);
            fine(2 * i + 1, 2 * j) = 0.5 * (coarse(i, j) + coarse(i + 1, j));
            fine(2 * i, 2 * j + 1) = 0.5 * (coarse(i, j) + coarse(i, j + 1));
            fine(2 * i + 1, 2 * j + 1) =
                0.25 * (coarse(i, j) + coarse(i + 1, j) + coarse(i, j + 1) + coarse(i + 1, j + 1));
        }
    }

    for (int i = 1; i <= imax; i++) {
        fine(2 * i - 1, 0) = coarse(i, 0);
        fine(2 * i, 0) = coarse(i, 0);
        fine(2 * i - 1, jmax + 1) = coarse(i, jmax + 1);
        fine(2 * i, jmax + 1) = coarse(i, jmax + 1);
    }

    for (int j = 1; j <= jmax; j++) {
        fine(0, 2 * j - 1) = coarse(0, j);
        fine(0, 2 * j) = coarse(0, j);
        fine(imax + 1, 2 * j - 1) = coarse(imax + 1, j);
        fine(imax + 1, 2 * j) = coarse(imax + 1, j);
    }

    return fine;
}