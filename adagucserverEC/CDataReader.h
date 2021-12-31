/******************************************************************************
 *
 * Project:  ADAGUC Server
 * Purpose:  ADAGUC OGC Server
 * Author:   Maarten Plieger, plieger "at" knmi.nl
 * Date:     2013-06-01
 *
 ******************************************************************************
 *
 * Copyright 2013, Royal Netherlands Meteorological Institute (KNMI)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#ifndef CDataReader_H
#define CDataReader_H
#include <math.h>
#include "CDebugger.h"
#include "CDataSource.h"
#include "CServerError.h"
#include "CDirReader.h"

//#include "CADAGUC_time.h"
#include "CCDFDataModel.h"
#include "CCDFNetCDFIO.h"

#include "CProj4ToCF.h"
#include "CStopWatch.h"
#include <sys/stat.h>

#include "CDFObjectStore.h"
#include "CCache.h"

#include "CAutoConfigure.h"
class CDataReader {
private:
  DEF_ERRORFUNCTION();
  bool _enableReporting;
  /**
   * Copies CRS info from the config when the "Projection" tag is present in the ADAGUC config.
   * If either of the id or the proj4 string is not defined, default values are copied.
   *
   * Returns false when the "Projection" tag is not present and nothing will be copied, otherwise true.
   */
  bool copyCRSFromConfigToDataSource(CDataSource *dataSource) const;

  /**
   * Copies the EPSG-code and the proj4 string of the geographical coordinate system (latitude, longitude)
   * into the datasource.
   */
  void copyLatLonCRS(CDataSource *dataSource) const;

  /**
   * Tries to copy the CRS info from a projection variable if it is present.
   * The method checks first if it can copy it according to the ADAGUC metadata standard.
   * Otherwise it tries to copy the CRS according to the CF conventions.
   *
   * Returns false when it was not possible to determine CRS info from a projection variable, otherwise true.
   */
  bool copyCRSFromProjectionVariable(CDataSource *dataSource) const;

  /**
   * Tries to copy the CRS info from the specified projection variable according to the ADAGUC metadata standard.
   *
   * Returns false when it was not possible to determine CRS info from the projection variable, otherwise true.
   */
  bool copyCRSFromADAGUCProjectionVariable(CDataSource *dataSource, const CDF::Variable *projVar) const;

  /**
   * Tries to copy the CRS info from the specified projection variable according to the CF conventions.
   * If this works, it adds the autogenerated proj4 string as "autogen_proj" attribute to the projection variable.
   *
   * Returns false when it was not possible to determine CRS info from the projection variable, otherwise true.
   */
  bool copyCRSFromCFProjectionVariable(CDataSource *dataSource, CDF::Variable *projVar) const;

  /**
   * Copies the EPSG code from the given projection variable.
   * If no EPSG code is found, creates a EPSG code based on the proj4 string in the datasource.
   */
  void copyEPSGCodeFromProjectionVariable(CDataSource *dataSource, const CDF::Variable *projVar) const;

  /**
   * Determines the X and Y dimension indices and copies them to the datasource.
   * If the dimension on the X position contains 'y' or 'lat', the indices are swapped.
   */
  void determineXAndYDimIndices(CDataSource *dataSource, const CDF::Variable *dataSourceVar) const;

  /**
   * Determines the X and Y variables and copies them to the datasource.
   * Returns false when it is not possible to find either the X or Y variable.
   */
  bool determineXandYVars(CDataSource *dataSource, const CDF::Variable *dataSourceVar, CDFObject *cdfObject) const;

  /**
   * Determines the value of stride2D map based on the compatibility mode and the configuration.
   */
  void determineStride2DMap(CDataSource *dataSource) const;

  /**
   * Determines the width and height based on stride. The width and height can be adjusted by passing a gridExtent.
   * When singleCellMode equals true, the width and height are set to a single cell.
   */
  void determineDWidthAndDHeight(CDataSource *dataSource, const bool singleCellMode, const int *gridExtent, int mode) const;

  /**
   * Calculates the cell size and bounding box based on the x and y variables.
   * The bounding box is calculated in a different order when the x variable is named 'col'.
   */
  bool calculateCellSizeAndBBox(CDataSource *dataSource, const CDF::Variable *dataSourceVar) const;

public:
  CDataReader() { _enableReporting = true; }
  ~CDataReader() {}
  int open(CDataSource *dataSource, int mode, int x, int y);
  int openExtent(CDataSource *dataSource, int mode, int *gridExtent);
  int open(CDataSource *dataSource, int mode, int x, int y, int *gridExtent);
  int open(CDataSource *dataSource, int x, int y);
  int open(CDataSource *dataSource, int mode);
  int open(CDataSource *dataSource, int mode, int *gridExtent);
  int parseDimensions(CDataSource *dataSource, int mode, int x, int y, int *gridExtent);
  int getCRS(CDataSource *dataSource);
  void enableReporting(bool enableReporting) { _enableReporting = enableReporting; }

  int close() { return 0; };

  /**
   * Returns the time dim for given datasource
   * @param dataSource the datasource
   * @return NULL if not available, otherwise a CDF::Variable.
   */
  static CDF::Variable *getTimeDimension(CDataSource *dataSource);

  /**
   * Get time units for this datasource, throws exception of int when failed.
   * @param dataSource
   * @return time units for this datasource
   */
  // DEPRECATED
  CT::string getTimeUnit(CDataSource *dataSource);

  /**
   * Possible dimension types
   */
  enum DimensionType { dtype_none, dtype_normal, dtype_time, dtype_reference_time, dtype_member, dtype_elevation };

  /**
   * Get the dimension type (time, elevation, member) by netcdf dimension name
   * @param cdfObject, the CDFObject belonging to the dimension
   * @param ncname the name dimension to check
   * @return DimensionType
   */
  static CDataReader::DimensionType getDimensionType(CDFObject *cdfObject, const char *ncname);

  /**
   * Get the dimension type (time, elevation, member) by CDF Variable
   * @param cdfObject, the CDFObject belonging to the dimension
   * @param variable the variable to check
   * @return DimensionType
   */
  static DimensionType getDimensionType(CDFObject *cdfObject, CDF::Variable *variable);

  /**
   * Get the dimension type (time, elevation, member of a Dimension object
   * @param cdfObject, the CDFObject belonging to the dimension
   * @param dimension the dimension to check
   * @return DimensionType
   */
  static CDataReader::DimensionType getDimensionType(CDFObject *cdfObject, CDF::Dimension *dimension);

  /**
   * Return the dimension matching to the requested dimensiontype within a variable
   * @param var The CDF::Variable containing the dimensions to query
   * @param dimensionType The dimension type to search for
   * @return NULL if not found, or the CDF::Dimension matching the query
   */
  static CDF::Dimension *searchDimensionByType(CDF::Variable *var, CDataReader::DimensionType dimensionType);

  /**
   * Same as searchDimensionByType, but returns the dimension variable instead of the dimension.
   * @param var The CDF::Variable containing the dimensions to query
   * @param dimensionType The dimension type to search for
   * @return NULL if not found, or the CDF::Dimension matching the query
   */
  static CDF::Variable *getDimensionVariableByType(CDF::Variable *var, CDataReader::DimensionType dimensionType);

  static CDF::Variable *addBlankDimVariable(CDFObject *cdfObject, const char *dimName);
};

#endif
