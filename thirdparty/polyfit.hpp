#pragma once

#ifdef BOOST_UBLAS_TYPE_CHECK
#	undef BOOST_UBLAS_TYPE_CHECK
#endif
#define BOOST_UBLAS_TYPE_CHECK 0
#ifndef _USE_MATH_DEFINES
#	define _USE_MATH_DEFINES
#endif

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <vector>
#include <stdexcept>

/*
	Finds the coefficients of a polynomial p(x) of degree n that fits the data, 
	p(x(i)) to y(i), in a least squares sense. The result p is a row vector of 
	length n+1 containing the polynomial coefficients in incremental powers.

	param:
		oX				x axis values
		oY				y axis values
		nDegree			polynomial degree including the constant

	return:
		coefficients of a polynomial starting at the constant coefficient and
		ending with the coefficient of power to nDegree. C++0x-compatible 
		compilers make returning locally created vectors very efficient.

*/
template<typename T>
std::vector<T> polyfit( const std::vector<T>& oX, const std::vector<T>& oY, int nDegree )
{
	using namespace boost::numeric::ublas;

	if ( oX.size() != oY.size() )
		throw std::invalid_argument( "X and Y vector sizes do not match" );

	// more intuative this way
	nDegree++;
	
	size_t nCount =  oX.size();
	matrix<T> oXMatrix( nCount, nDegree );
	matrix<T> oYMatrix( nCount, 1 );
	
	// copy y matrix
	for ( size_t i = 0; i < nCount; i++ )
	{
		oYMatrix(i, 0) = oY[i];
	}

	// create the X matrix
	for ( size_t nRow = 0; nRow < nCount; nRow++ )
	{
		T nVal = 1.0f;
		for ( int nCol = 0; nCol < nDegree; nCol++ )
		{
			oXMatrix(nRow, nCol) = nVal;
			nVal *= oX[nRow];
		}
	}

	// transpose X matrix
	matrix<T> oXtMatrix( trans(oXMatrix) );
	// multiply transposed X matrix with X matrix
	matrix<T> oXtXMatrix( prec_prod(oXtMatrix, oXMatrix) );
	// multiply transposed X matrix with Y matrix
	matrix<T> oXtYMatrix( prec_prod(oXtMatrix, oYMatrix) );

	// lu decomposition
	permutation_matrix<int> pert(oXtXMatrix.size1());
	const std::size_t singular = lu_factorize(oXtXMatrix, pert);
	// must be singular
	BOOST_ASSERT( singular == 0 );

	// backsubstitution
	lu_substitute(oXtXMatrix, pert, oXtYMatrix);

	// copy the result to coeff
	return std::vector<T>( oXtYMatrix.data().begin(), oXtYMatrix.data().end() );
}

/*
	Calculates the value of a polynomial of degree n evaluated at x. The input 
	argument pCoeff is a vector of length n+1 whose elements are the coefficients 
	in incremental powers of the polynomial to be evaluated.

	param:
		oCoeff			polynomial coefficients generated by polyfit() function
		oX				x axis values

	return:
		Fitted Y values. C++0x-compatible compilers make returning locally 
		created vectors very efficient.
*/
template<typename T>
std::vector<T> polyval( const std::vector<T>& oCoeff, const std::vector<T>& oX )
{
	size_t nCount =  oX.size();
	size_t nDegree = oCoeff.size();
	std::vector<T>	oY( nCount );

	for ( size_t i = 0; i < nCount; i++ )
	{
		T nY = 0;
		T nXT = 1;
		T nX = oX[i];
		for ( size_t j = 0; j < nDegree; j++ )
		{
			// multiply current x by a coefficient
			nY += oCoeff[j] * nXT;
			// power up the X
			nXT *= nX;
		}
		oY[i] = nY;
	}

	return oY;
}

#include "matrix.hpp"
#include "GivensQR.hpp"
/*
Finds the coefficients of a polynomial p(x) of degree n that fits the data,
p(x(i)) to y(i), in a least squares sense. The result p is a row vector of
length n+1 containing the polynomial coefficients in incremental powers.

param:
oX				x axis values
oY				y axis values
nDegree			polynomial degree including the constant

return:
coefficients of a polynomial starting at the constant coefficient and
ending with the coefficient of power to nDegree. C++0x-compatible
compilers make returning locally created vectors very efficient.

*/
template<typename T>
std::vector<T> polyfitqr(const std::vector<T>& oX, const std::vector<T>& oY, int nDegree) {
  if (oX.size() != oY.size())
    throw std::invalid_argument("X and Y vector sizes do not match");

  // more intuative this way
  nDegree++;

  size_t nCount = oX.size();
  mathalgo::matrix<T> oXMatrix(nCount, nDegree);
  mathalgo::matrix<T> oYMatrix(nCount, 1);

  // copy y matrix
  for (size_t i = 0; i < nCount; i++) {
    oYMatrix(i, 0) = oY[i];
  }

  // create the X matrix
  for (size_t nRow = 0; nRow < nCount; nRow++) {
    T nVal = 1.0f;
    for (int nCol = 0; nCol < nDegree; nCol++) {
      oXMatrix(nRow, nCol) = nVal;
      nVal *= oX[nRow];
    }
  }

  // transpose X matrix
  mathalgo::matrix<T> oXtMatrix(oXMatrix.transpose());
  // multiply transposed X matrix with X matrix
  mathalgo::matrix<T> oXtXMatrix(oXtMatrix * oXMatrix);
  // multiply transposed X matrix with Y matrix
  mathalgo::matrix<T> oXtYMatrix(oXtMatrix * oYMatrix);

  mathalgo::Givens<T> oGivens;
  oGivens.Decompose(oXtXMatrix);
  mathalgo::matrix<T> oCoeff = oGivens.Solve(oXtYMatrix);
  // copy the result to coeff
  return oCoeff.data();
}

/*
Calculates the value of a polynomial of degree n evaluated at x. The input
argument pCoeff is a vector of length n+1 whose elements are the coefficients
in incremental powers of the polynomial to be evaluated.

param:
oCoeff			polynomial coefficients generated by polyfit() function
oX				x axis values

return:
Fitted Y values. C++0x-compatible compilers make returning locally
created vectors very efficient.
*/
template<typename T>
std::vector<T> polyvalqr(const std::vector<T>& oCoeff, const std::vector<T>& oX) {
  size_t nCount = oX.size();
  size_t nDegree = oCoeff.size();
  std::vector<T>	oY(nCount);

  for (size_t i = 0; i < nCount; i++) {
    T nY = 0;
    T nXT = 1;
    T nX = oX[i];
    for (size_t j = 0; j < nDegree; j++) {
      // multiply current x by a coefficient
      nY += oCoeff[j] * nXT;
      // power up the X
      nXT *= nX;
    }
    oY[i] = nY;
  }

  return oY;
}

template<typename T>
mathalgo::matrix<T> combineXY(std::vector<T>& oX, std::vector<T>& oY) {
  size_t nCount = oX.size();
  size_t nCols = 3 * 2; // 3 unknowns in 2 dimensions
  mathalgo::matrix<T> oXYMatrix(nCount, nCols);
  
  // normalize x and y matrix
  for (size_t i = 1; i < nCount; i++) {
    oX[i] = oX[i] - oX[0];
    oY[i] = oY[i] - oY[0];
  }
  oX[0] = 0;
  oY[0] = 0;

  // create the XY matrix
  for (size_t nRow = 0; nRow < nCount; nRow++) {
    oXYMatrix(nRow, 0) = 1;
    oXYMatrix(nRow, 1) = oX[nRow];
    oXYMatrix(nRow, 2) = oY[nRow];
    oXYMatrix(nRow, 3) = oX[nRow] * oY[nRow];
    oXYMatrix(nRow, 4) = std::pow(oX[nRow], 2);
    oXYMatrix(nRow, 5) = std::pow(oY[nRow], 2);

    //int nCol = 0;
    //for (size_t i = 0; i < nDegree; i++) {
    //  for (size_t j = 0; j < nDegree; j++) {
    //    oXYMatrix(nRow, i*nDegree+j) = std::pow(oX[nRow], i) * std::pow(oY[nRow], j);
    //    nCol++;
    //  }
    //}
  }
  return oXYMatrix;
}

// 3D plane fitting
template<typename T>
std::vector<T> polyfit3d(std::vector<T>& oX, std::vector<T>& oY, std::vector<T>& oZ, std::vector<T>& calculated) {
  if (oX.size() != oY.size() || oX.size() != oZ.size())
    throw std::invalid_argument("X and Y or X and Z vector sizes do not match");

  size_t nCount = oX.size();
  mathalgo::matrix<T> oXYMatrix(combineXY(oX, oY));
  mathalgo::matrix<T> oZMatrix(nCount, 1);

  // copy z matrix
  for (size_t i = 0; i < nCount; i++) {
    oZMatrix(i, 0) = oZ[i];
  }

  // transpose X matrix
  mathalgo::matrix<T> oXYtMatrix(oXYMatrix.transpose());
  // multiply transposed X matrix with X matrix
  mathalgo::matrix<T> oXYtXYMatrix(oXYtMatrix * oXYMatrix);
  // multiply transposed X matrix with Y matrix
  mathalgo::matrix<T> oXYtZMatrix(oXYtMatrix * oZMatrix);

  mathalgo::Givens<T> oGivens;
  oGivens.Decompose(oXYtXYMatrix);
  mathalgo::matrix<T> oCoeff = oGivens.Solve(oXYtZMatrix);

  calculated = polyval3d(oCoeff.data(), oXYMatrix);
  // copy the result to coeff
  return oCoeff.data();
}

template<typename T>
std::vector<T> polyval3d(std::vector<T>& oCoeff, mathalgo::matrix<T>& oXY) {
  size_t nCount = oXY.rows();
  size_t nDegree = oCoeff.size();
  mathalgo::matrix<T> oCoeffMatrix(oCoeff.size(), 1);

  std::vector<T> oZ;

  // copy coeffs matrix
  for (size_t i = 0; i < nDegree; i++) {
    oCoeffMatrix(i, 0) = oCoeff[i];
  }

  for (size_t i = 0; i < oXY.rows(); i++) {
    // transpose Y matrix
    mathalgo::matrix<T> oYtMatrix(oXY.row(i));
    // multiply transposed Y matrix with Z matrix
    mathalgo::matrix<T> oYtZMatrix(oYtMatrix * oCoeffMatrix);
    oZ.push_back(oYtZMatrix(0, 0));
  }

  return oZ;
}