/***********************************************************************
XDMFFileReader - Functions to read Xdmf files.
Copyright (c) 2005-2006 Oliver Kreylos

This file is part of the Format Handling Library (Files).

The Format Handling Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Format Handling Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Format Handling Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef FILES_XDMFFILEREADER_INCLUDED
#define FILES_XDMFFILEREADER_INCLUDED

namespace Files {

template <class DataSourceParam>
class XDMFFileReader 
   {
   /* Embedded classes: */
   public:
   typedef DataSourceParam DataSource; // Type of file data source
   private:

   /* Elements */
   private:
   DataSource& dataSource;

   /* Constructors and destructors: */
   public:
   XDMFFileReader(DataSource& sDataSource); // Creates an XDMF file reader for the given data source
   ~XDMFFileReader(void);
   
   };

}

#endif
