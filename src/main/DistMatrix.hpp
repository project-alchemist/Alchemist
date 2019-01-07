#ifndef ALCHEMIST__DISTMATRIX_HPP
#define ALCHEMIST__DISTMATRIX_HPP

#include <El.hpp>
#include "mpi.h"

namespace alchemist {

struct DistMatrix {

	DistMatrix(uint64_t num_rows, uint64_t num_cols, El::Grid & _grid) : grid(_grid) {
		data = std::make_shared<El::DistMatrix<double, El::VR, El::STAR>>(El::DistMatrix<double, El::VR, El::STAR>(num_rows, num_cols, grid));
		El::Zero(*data);
	}

	virtual ~DistMatrix() { }

	std::shared_ptr<El::DistMatrix<double, El::VR, El::STAR>> data;

	El::Grid & grid;
};

typedef std::shared_ptr<El::Grid> Grid_ptr;
typedef std::shared_ptr<DistMatrix> DistMatrix_ptr;

}

#endif
