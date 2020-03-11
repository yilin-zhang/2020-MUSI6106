
#include "Vector.h"
#include "Util.h"

#include "Dtw.h"
#include <algorithm>

CDtw::CDtw( void ) : m_bIsInitialized(false),
                     m_bIsProcessed(false),
                     m_iNumRows(0),
                     m_iNumCols(0),
                     m_ppfCost(nullptr),
                     m_ppkDelta(nullptr),
                     m_iPathLength(0)
{

}

CDtw::~CDtw( void )
{

}

Error_t CDtw::init( int iNumRows, int iNumCols )
{
    if (iNumCols <= 0 || iNumRows <= 0)
        return kFunctionInvalidArgsError;

    m_iNumRows = iNumRows;
    m_iNumCols = iNumCols;

    m_ppfCost   = new float*[iNumRows];
    for (int i = 0; i < iNumRows; i++) {
        m_ppfCost[i]    = new float [iNumCols];
        for (int j = 0; j < iNumCols; j++)
            m_ppfCost[i][j] = 0;
    }

    m_ppkDelta   = new int*[iNumRows];
    for (int i = 0; i < iNumRows; i++) {
        m_ppkDelta[i]    = new int [iNumCols];
        for (int j = 0; j < iNumCols; j++)
            m_ppkDelta[i][j] = 0;
    }

    m_bIsInitialized = true;

    return kNoError;
}

Error_t CDtw::reset()
{
    for (int i = 0; i < m_iNumRows; i++)
        delete[] m_ppfCost[i];
    delete[] m_ppfCost;
    m_ppfCost = nullptr;

    for (int i = 0; i < m_iNumRows; i++)
        delete[] m_ppkDelta[i];
    delete[] m_ppkDelta;
    m_ppkDelta = nullptr;

    m_iNumRows = 0;
    m_iNumCols = 0;
    m_iPathLength = 0;

    m_bIsInitialized = false;
    m_bIsProcessed = false;

    return kNoError;
}

Error_t CDtw::process(float **ppfDistanceMatrix)
{
    if (!m_bIsInitialized)
        return kNotInitializedError;
    if (ppfDistanceMatrix == nullptr)
        return kFunctionInvalidArgsError;
    float minimum = 0;
    // initialize the first row
    for (int i = 0; i < m_iNumCols; i++) {
        if (i == 0)
            m_ppfCost[0][i] = ppfDistanceMatrix[0][i];
        else
            m_ppfCost[0][i] = m_ppfCost[0][i-1] + ppfDistanceMatrix[0][i];
        m_ppkDelta[0][i] = kHoriz;
    }
    // initialize the first col
    for (int i = 1; i < m_iNumRows; i++) {
        m_ppfCost[i][0] = m_ppfCost[i-1][0] + ppfDistanceMatrix[i][0];
        m_ppkDelta[i][0] = kVert;
    }
    // update the rest of the elements
    for (int i = 1; i < m_iNumRows; i++) {
        for (int j = 1; j < m_iNumCols; j++) {
            minimum = std::min(m_ppfCost[i][j-1],
                                       std::min(m_ppfCost[i-1][j],
                                                m_ppfCost[i-1][j-1]));
            m_ppfCost[i][j] = ppfDistanceMatrix[i][j] + minimum;
            if (minimum == m_ppfCost[i][j-1])
                m_ppkDelta[i][j] = kHoriz;
            else if (minimum == m_ppfCost[i-1][j])
                m_ppkDelta[i][j] = kVert;
            else if (minimum == m_ppfCost[i-1][j-1])
                m_ppkDelta[i][j] = kDiag;
            }
        }

    m_bIsProcessed = true;
    return kNoError;
}

int CDtw::getPathLength()
{
    if (!m_bIsInitialized)
        return 0;
    if (!m_bIsProcessed)
        return 0;

    int i = m_iNumRows - 1;
    int j = m_iNumCols - 1;
    int pathLength = 0;
    while (i >= 0 && j >= 0) {
        if (m_ppkDelta[i][j] == kHoriz)
            j--;
        else if (m_ppkDelta[i][j] == kVert)
            i--;
        else if (m_ppkDelta[i][j] == kDiag) {
            i--;
            j--;
        }
        pathLength++;
    }
    m_iPathLength = pathLength;
    return pathLength;
}

float CDtw::getPathCost() const
{
    return m_ppfCost[m_iNumRows - 1][m_iNumCols - 1];
}

Error_t CDtw::getPath( int **ppiPathResult ) const
{
    if (!m_bIsProcessed)
        return kNotInitializedError;

    int i = m_iNumRows - 1;
    int j = m_iNumCols - 1;
    int idx = m_iPathLength - 1;
    while (idx >= 0) {
        ppiPathResult[0][idx] = i;
        ppiPathResult[1][idx] = j;
        if (m_ppkDelta[i][j] == kHoriz)
            j--;
        else if (m_ppkDelta[i][j] == kVert)
            i--;
        else if (m_ppkDelta[i][j] == kDiag) {
            i--;
            j--;
        }
        idx--;
    }
    return kNoError;
}

