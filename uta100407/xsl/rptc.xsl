<?xml version="1.0"?>

<xsl:transform version = "1.0" id="dftc"
    extension-element-prefixes="uta"
    xmlns:xsl = "http://www.w3.org/1999/XSL/Transform"
    xmlns:uta = "http://www.ingenico.com/xml/uta"
>

<xsl:include href="inc.xsl"/>

<xsl:template match="/">
  <xsl:text>#include "str.h"&#010;</xsl:text>
  <xsl:text>&#010;</xsl:text>

  <xsl:text>//This file is automatically generated from "dbs.xml" file&#010;</xsl:text>
  <xsl:text>//Don't modify it manually!&#010;</xsl:text>
  <xsl:text>&#010;</xsl:text>

  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="uta:dbk"/>

<xsl:template match="uta:rptLayout">
  <xsl:text>typedef struct sLayout{ //one line layout&#010;</xsl:text>
  <xsl:text>    const char *esc; //escape characters to modify font&#010;</xsl:text>
  <xsl:text>    int key; //database key of the data element to be printed (0= terminator; negative= rlo block)&#010;</xsl:text>
  <xsl:text>} tLayout;&#010;</xsl:text>
  <xsl:text>&#010;</xsl:text>

  <!-- Build rptSection() function -->
  <xsl:text>/** Print an array of lines.&#010;</xsl:text>
  <xsl:text> * \param  rpt (I) array of line descriptors.&#010;</xsl:text>
  <xsl:text> * \return&#010;</xsl:text>
  <xsl:text> * - 1 if OK&#010;</xsl:text>
  <xsl:text> * - negative if failure&#010;</xsl:text>
  <xsl:text> * &#010;</xsl:text>
  <xsl:text> * Each line is described by a number of escape characters that control printing&#010;</xsl:text>
  <xsl:text> * and UTA data base key of a data element to be printed.&#010;</xsl:text>
  <xsl:text> * &#010;</xsl:text>
  <xsl:text> * If a key of line is negative it is interpreted as key of block in receipt layout.&#010;</xsl:text>
  <xsl:text> * If a block key is out of the receipt layout range the uta:default attribute&#010;</xsl:text>
  <xsl:text> * value is interpreted as a function name to be called to print the section.&#010;</xsl:text>
  <xsl:text> * &#010;</xsl:text>
  <xsl:text> * Escape characters are sent to printer driver directly&#010;</xsl:text>
  <xsl:text> * with the exception of 'i' character that is considered as an image printing.&#010;</xsl:text>
  <xsl:text> */&#010;</xsl:text>
  <xsl:text>static int rptSection(const tLayout *rpt){&#010;</xsl:text>
  <xsl:text>    int ret;&#010;</xsl:text>
  <xsl:text>    word tmp;&#010;</xsl:text>
  <xsl:text>    byte buf[255];&#010;</xsl:text>
  <xsl:text>&#010;</xsl:text>
  <xsl:text>    VERIFY(rpt);&#010;</xsl:text>
  <xsl:text>    while(rpt->key){&#010;</xsl:text>
  <xsl:text>        if(rpt->key>0){&#010;</xsl:text>
  <xsl:text>            tmp= rpt->key;&#010;</xsl:text>
  <xsl:text>            ret= mapGet(tmp,buf,255); CHECK(ret>=0,lblKO);&#010;</xsl:text>
  <xsl:text>            if(*buf){&#010;</xsl:text>
  <xsl:text>                if(rpt->esc){&#010;</xsl:text>
  <xsl:text>                    if(rpt->esc[0]=='i'){&#010;</xsl:text>
  <xsl:text>                        ret= prtImg(buf+3,buf[1],buf[2],rpt->esc+1);&#010;</xsl:text>
  <xsl:text>                    }else{&#010;</xsl:text>
  <xsl:text>                        ret= prtES(rpt->esc,(char *)buf);&#010;</xsl:text>
  <xsl:text>                    }&#010;</xsl:text>
  <xsl:text>                }else&#010;</xsl:text>
  <xsl:text>                    ret= prtS((char *)buf);&#010;</xsl:text>
  <xsl:text>            }&#010;</xsl:text>
  <xsl:text>        }else{&#010;</xsl:text>
  <xsl:text>            tmp= -rpt->key;&#010;</xsl:text>
  <xsl:text>            ret= rptReceipt(tmp); CHECK(ret>=0,lblKO);&#010;</xsl:text>
  <xsl:text>        }&#010;</xsl:text>
  <xsl:text>        CHECK(ret>=0,lblKO);&#010;</xsl:text>
  <xsl:text>        rpt++;&#010;</xsl:text>
  <xsl:text>    }&#010;</xsl:text>
  <xsl:text>    return 1;&#010;</xsl:text>
  <xsl:text>lblKO:&#010;</xsl:text>
  <xsl:text>    return -1;&#010;</xsl:text>
  <xsl:text>}&#010;</xsl:text>
  <xsl:text>&#010;</xsl:text>

  <xsl:apply-templates mode="layout"/>
  
  <!-- Build rptReceipt() function -->
  <xsl:text>/** Print a receipt layout block.&#010;</xsl:text>
  <xsl:text> * \param  rlo (I) key to access receipt layout block.&#010;</xsl:text>
  <xsl:text> * \return&#010;</xsl:text>
  <xsl:text> * - 1 if OK&#010;</xsl:text>
  <xsl:text> * - negative if failure&#010;</xsl:text>
  <xsl:text> * &#010;</xsl:text>
  <xsl:text> * If rlo key is out of the receipt layout range the function is called &#010;</xsl:text>
  <xsl:text> * defined in uta:default attribute.&#010;</xsl:text>
  <xsl:text> */&#010;</xsl:text>
  <xsl:text>int rptReceipt(word rlo){&#010;</xsl:text>
  <!--
  <xsl:text>    VERIFY(rloBeg&lt;rlo);&#010;</xsl:text>
  <xsl:text>    VERIFY(rlo&lt;rloEnd);&#010;</xsl:text>
  -->
  <xsl:text>    switch(rlo){&#010;</xsl:text>
  <xsl:apply-templates mode="rptReceipt"/>    
  <xsl:text>        default: break;&#010;</xsl:text>
  <xsl:text>    }&#010;</xsl:text>
  <xsl:text>    return </xsl:text>
  <xsl:choose>
    <xsl:when test="@uta:default!=''">
      <xsl:value-of select="@uta:default"/>
      <xsl:text>(rlo)</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>-1</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:text>;&#010;</xsl:text>
  <xsl:text>}&#010;</xsl:text>
  <xsl:text>&#010;</xsl:text>


</xsl:template>

<xsl:template match="uta:section" mode="layout">
    <xsl:text>static tLayout sec</xsl:text>
    <xsl:value-of select="translate(substring(@uta:id,1,1),$az,$AZ)"/>
    <xsl:value-of select="substring(@uta:id,2)"/>
    <xsl:text>[]= {&#010;</xsl:text>

    <xsl:apply-templates mode="layout"/>

    <xsl:text>    {0, 0}&#010;</xsl:text>
    <xsl:text>};&#010;</xsl:text>
    <xsl:text>&#010;</xsl:text>
</xsl:template>

<xsl:template match="uta:line" mode="layout">
    <xsl:text>    {</xsl:text>
    <xsl:choose>
        <xsl:when test="@uta:esc!=''">
            <xsl:value-of select="@uta:esc"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:text>0</xsl:text>
        </xsl:otherwise>
    </xsl:choose>
    <xsl:text>, </xsl:text>
    <xsl:value-of select="text()"/>
    <xsl:text>},&#010;</xsl:text>
</xsl:template>

<xsl:template match="uta:block" mode="layout">
    <xsl:text>    {0, -(</xsl:text>
    <xsl:value-of select="text()"/>
    <xsl:text>)},&#010;</xsl:text>
</xsl:template>

<xsl:template match="uta:section" mode="rptReceipt">
    <xsl:text>        case </xsl:text>
    <xsl:value-of select="@uta:id"/>
    <xsl:text>: return rptSection(sec</xsl:text>
    <xsl:value-of select="translate(substring(@uta:id,1,1),$az,$AZ)"/>
    <xsl:value-of select="substring(@uta:id,2)"/>
    <xsl:text>);&#010;</xsl:text>
</xsl:template>

<xsl:template match="uta:keySpace"/>
<xsl:template match="uta:keySpace" mode="layout"/>
<xsl:template match="uta:keySpace" mode="rptReceipt"/>

</xsl:transform>
