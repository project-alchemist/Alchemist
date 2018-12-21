#ifndef ALCHEMIST__DISTMATRIX_HPP
#define ALCHEMIST__DISTMATRIX_HPP

#include <El.hpp>
#include "mpi.h"

namespace alchemist {

//struct DistMatrix {
//
//	MPI_Comm & peers;
//	El::Grid grid;
//};

struct DistMatrix {

//	DistMatrix(uint64_t num_rows, uint64_t num_cols, MPI_Comm & _peers) : peers(_peers), grid(El::mpi::Comm(_peers)) {
//		data = std::make_shared<El::DistMatrix<double, El::VR, El::STAR>>(El::DistMatrix<double, El::VR, El::STAR>(num_rows, num_cols, grid));
//		El::Zero(*data);
//	}

	DistMatrix(uint64_t num_rows, uint64_t num_cols, El::Grid & _grid) : grid(_grid) {
		data = std::make_shared<El::DistMatrix<double, El::VR, El::STAR>>(El::DistMatrix<double, El::VR, El::STAR>(num_rows, num_cols, grid));
		El::Zero(*data);
	}

	virtual ~DistMatrix() { }

	std::shared_ptr<El::DistMatrix<double, El::VR, El::STAR>> data;

//	MPI_Comm & peers;
	El::Grid & grid;
};

//struct DistMatrix_STAR_STAR : DistMatrix {
//
//	DistMatrix_STAR_STAR(uint64_t num_rows, uint64_t num_cols, MPI_Comm & _peers) : peers(_peers), grid(El::mpi::Comm(_peers)) {
//		data = std::make_shared<El::DistMatrix<double, El::STAR, El::STAR>>(El::DistMatrix<double, El::STAR, El::STAR>(num_rows, num_cols, grid));
//		El::Zero(*data);
//	}
//
//	virtual ~DistMatrix_STAR_STAR() { }
//
//	std::shared_ptr<El::DistMatrix<double, El::STAR, El::STAR>> data;
//};


typedef std::shared_ptr<El::Grid> Grid_ptr;
typedef std::shared_ptr<DistMatrix> DistMatrix_ptr;

}

#endif
