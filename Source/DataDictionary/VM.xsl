<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text" indent="yes"/>
<!-- share common code to transform a VM Part 6 string into a gdcm::VM type
-->
<!--
  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2008 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
-->
  <xsl:template name="VMStringToVMType">
<!-- FIXME: Supid function I could simple translate('-','_') + concat ... -->
    <xsl:param name="vmstring"/>
    <xsl:choose>
      <xsl:when test="$vmstring = ''">
        <xsl:value-of select="'VM0'"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="concat('VM',translate($vmstring,'-','_'))"/>
      </xsl:otherwise>
    </xsl:choose>
 </xsl:template>
</xsl:stylesheet>
