// bdf2xml.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
using namespace std;

enum eErr{
	errBeg,
	errUsage,
	errOpenSrc,
	errOpenDst,
	errEnd
};

static string indent= "";

//http://netghost.narod.ru/gff/graphics/summary/bdf.htm#BDF-DMYID.2
//http://netghost.narod.ru/gff/vendspec/bdf/index.htm
//5005.BDF_Spec.pdf: Glyph Bitmap Distribution Format (BDF) Specification, Version 2.2, 22 March 1993
const string tagLINE= "LINE";
const string tagSTARTFONT= "STARTFONT";
const string tagENDFONT= "ENDFONT";
const string tagCOMMENT= "COMMENT";
const string tagFONT= "FONT";
const string tagSIZE= "SIZE";
const string tagFONTBOUNDINGBOX= "FONTBOUNDINGBOX";
const string tagSTARTPROPERTIES= "STARTPROPERTIES";
const string tagENDPROPERTIES= "ENDPROPERTIES";
const string tagCHARS= "CHARS";
const string tagSTARTCHAR= "STARTCHAR";
const string tagENDCHAR= "ENDCHAR";
const string tagENCODING= "ENCODING";
const string tagSWIDTH= "SWIDTH";
const string tagDWIDTH= "DWIDTH";
const string tagBBX= "BBX";
const string tagBITMAP= "BITMAP";

static int startsWith(const string& str,const string& pat){
	if(str==pat) return 1;
	if(str.substr(0,pat.length())!=pat) return 0;
	if(str[pat.length()]==' ') return 1;
	return 0;
}

static void parseLine(string &line,ofstream& dst){
	if(line=="") return;
	//dst << indent << "<" << tagLINE << " value='" << line << "'/>" << endl;
}

static void parseLineSTARTFONT(string &line,ofstream& dst){
	//STARTFONT <version>
	line.erase(0,tagSTARTFONT.length()+1);
	dst << indent << "<" << "BDFFONT" << " version='" << line << "'>" << endl;
	indent+= "  ";
}

static void parseLineENDFONT(string &line,ofstream& dst){
	indent.erase(0,2);
	dst << indent << "</" << "BDFFONT" << ">" << endl;
}

static void parseLineCOMMENT(string &line,ofstream& dst){
	//COMMENT <text>
	line.erase(0,tagCOMMENT.length()+1);
	dst << indent << "<" << tagCOMMENT << " text='" << line << "'/>" << endl;
}

static void parseLineFONT(string &line,ofstream& dst){
	//FONT <fontname>
	line.erase(0,tagFONT.length()+1);
	dst << indent << "<" << tagFONT << " fontname='" << line << "'/>" << endl;
}

static void parseLineSIZE(string &line,ofstream& dst){
	//SIZE <pointsize> <x resolution> <y resolution>
	line.erase(0,tagSIZE.length()+1);
	istringstream src(line);
	int pointsize,xresolution,yresolution;
	src >> pointsize >> xresolution >> yresolution;
	dst << indent << "<" << tagSIZE
		<< " pointsize ='" << pointsize << "' "
		<< " xresolution='" << xresolution << "' "
		<< " yresolution='" << yresolution << "'/>"
		<< endl;
}

static int width,height,xoffset,yoffset;
static void parseLineFONTBOUNDINGBOX(string &line,ofstream& dst){
	//FONTBOUNDINGBOX <width> <height> <x offset> <y offset>
	line.erase(0,tagFONTBOUNDINGBOX.length()+1);
	istringstream src(line);
	//int width,height,xoffset,yoffset;
	src >> width >> height >> xoffset >> yoffset;
	dst << indent << "<" << tagFONTBOUNDINGBOX
		<< " width='" << width << "' "
		<< " height='" << height << "' "
		<< " xoffset='" << xoffset << "' "
		<< " yoffset='" << yoffset << "'/>"
		<< endl;
}

static void parseLineSTARTPROPERTIES(string &line,ofstream& dst){
	//The STARTPROPERTIES keyword is followed by the number of properties defined within this record
	line.erase(0,tagSTARTPROPERTIES.length()+1);
	dst << indent << "<" << "PROPERTIES" << " numberOfProperties='" << line << "'>" << endl;
	indent+= "  ";
}

static void parseLineENDPROPERTIES(string &line,ofstream& dst){
	indent.erase(0,2);
	dst << indent << "</" << "PROPERTIES" << ">" << endl;
}

static void parseLineCHARS(string &line,ofstream& dst){
	//CHARS <number of segments>
	line.erase(0,tagCHARS.length()+1);
	dst << indent << "<" << tagCHARS << " numberOfSegements='" << line << "'/>" << endl;
}

static string glyphname= "";
static void parseLineSTARTCHAR(string &line,ofstream& dst){
	//STARTCHAR <glyphname>
	line.erase(0,tagSTARTCHAR.length()+1);
	glyphname= line;
	dst << indent << "<" << "CHAR" << " glyphname='" << glyphname << "'>" << endl;
	indent+= "  ";
}

static void parseLineENDCHAR(string &line,ofstream& dst){
	indent.erase(0,2);
	dst << indent << "</" << "CHAR" << ">" << endl;
}

static void parseLineENCODING(string &line,ofstream& dst){
	line.erase(0,tagENCODING.length()+1);
	dst << indent << "<" << tagENCODING << " glyphIndex='" << line << "'/>" << endl;
}

static void parseLineSWIDTH(string &line,ofstream& dst){
	line.erase(0,tagSWIDTH.length()+1);
	dst << indent << "<" << tagSWIDTH<< " scalableWidth='" << line << "'/>" << endl;
}

static void parseLineDWIDTH(string &line,ofstream& dst){
	line.erase(0,tagDWIDTH.length()+1);
	istringstream src(line);
	int xwidth,yheight;
	src >> xwidth >> yheight;
	dst << indent << "<" << tagDWIDTH
		<< " xwidth='" << xwidth << "' "
		<< " yheight='" << yheight << "'/>"
		<< endl;
}

static int bbw,bbh,bbox,bboy;
static int parseLineBBX(string &line,ofstream& dst){
	//BBX BBw BBh BBxoff0x BByoff0y
	line.erase(0,tagBBX.length()+1);
	istringstream src(line);
	//int bbw,bbh,bbox,bboy;
	src >> bbw >> bbh >> bbox >> bboy;
	dst << indent << "<" << tagBBX
		<< " bbw='" << bbw << "' "
		<< " bbh='" << bbh << "' "
		<< " bbox='" << bbox << "' "
		<< " bboy='" << bboy << "'/>"
		<< endl;
	return bbh;
}

static void parseLineBITMAP(string &line,ofstream& dst){
	line.erase(0,tagSTARTCHAR.length()+1);
	dst << indent << "<BITMAP"
//		<< " id='" << glyphname << "'"
		<< ">" << endl;
	indent+= "  ";
}

static string bitmap= "";
static void parseLineBMP(string &line,ofstream& dst){
	string bit;
	string nibble;
	bit= "";
	for(int i= 0; i<line.length(); i++){
		nibble= "";
		switch(line[i]){
			case '0': nibble= "...."; break;
			case '1': nibble= "...*"; break;
			case '2': nibble= "..*."; break;
			case '3': nibble= "..**"; break;
			case '4': nibble= ".*.."; break;
			case '5': nibble= ".*.*"; break;
			case '6': nibble= ".**."; break;
			case '7': nibble= ".***"; break;
			case '8': nibble= "*..."; break;
			case '9': nibble= "*..*"; break;
			case 'A': nibble= "*.*."; break;
			case 'B': nibble= "*.**"; break;
			case 'C': nibble= "**.."; break;
			case 'D': nibble= "**.*"; break;
			case 'E': nibble= "***."; break;
			case 'F': nibble= "****"; break;
		}
		bit+= nibble;
	}
	bit.erase(bbw,bit.length()-bbw);
	dst << indent << "<BMPLINE"
		<< " hex='" << line << "'"
		<< " bit='" << bit << "'"
		"/>" << endl;
	bitmap+= bit;
	bitmap+= " ";
}

static void saveBox(ofstream& dst){
	if(bitmap=="") return;
	dst << indent << "<BOX"
//		<< " id='" << glyphname << "'"
		<< " width='" << width << "'"
		<< " height='" << height << "'"
		<< " xoffset='" << xoffset << "'"
		<< " yoffset='" << yoffset << "'"
		">" << endl;
	indent += "  ";

	istringstream src(bitmap);
	for(int i= height+yoffset; i>yoffset; i--){
		string bit= "";;
		if(i>bbh+bboy || i<bboy){
			for(int j= 0; j<width; j++) bit+= ".";
		}else{
			src >> bit;
			for(int j= xoffset; j<bbox; j++) bit= string(".")+bit;
			while(bit.length()<width) bit+= ".";
		}

		string hex= "";
		string nibble= "";
		string tmp= bit;
		while(tmp.length()%8) tmp+= ".";
		for(int j= 0; j<tmp.length(); j++){
			nibble+= tmp[j];
			if(nibble.length()<4) continue;
			if(nibble=="....") hex+= "0";
			else if(nibble=="...*") hex+= "1";
			else if(nibble=="..*.") hex+= "2";
			else if(nibble=="..**") hex+= "3";
			else if(nibble==".*..") hex+= "4";
			else if(nibble==".*.*") hex+= "5";
			else if(nibble==".**.") hex+= "6";
			else if(nibble==".***") hex+= "7";
			else if(nibble=="*...") hex+= "8";
			else if(nibble=="*..*") hex+= "9";
			else if(nibble=="*.*.") hex+= "A";
			else if(nibble=="*.**") hex+= "B";
			else if(nibble=="**..") hex+= "C";
			else if(nibble=="**.*") hex+= "D";
			else if(nibble=="***.") hex+= "E";
			else if(nibble=="****") hex+= "F";
			nibble= "";
		}

		dst << indent << "<BOXLINE"
			<< " hex='" << hex << "'"
			<< " bit='" << bit << "'"
			"/>" << endl;
	}

	indent.erase(0,2);
	dst << indent << "</BOX>" << endl;
	bitmap= "";
}

int main(int argc, char* argv[]){
	int ret= 0;
	try{
		if(argc!=3) throw(errUsage);

		ifstream src(argv[1]);
		if(!src.is_open()) throw(errOpenSrc);

		ofstream dst(argv[2]);
		if(!dst.is_open()) throw(errOpenDst);

		string exeName= argv[0];
		int i= exeName.rfind("\\");
		if(i<0) i= 0; else i++;
		int j= exeName.rfind(".exe");
		if(j<0) j= exeName.length();
		exeName= exeName.substr(i,j-i);
		dst << "<?xml version=\"1.0\" encoding=\"us-ascii\"?>" << endl << endl;
		dst << "<!-- Generated from " << argv[1] << " by " << exeName << " -->" << endl;

		int h= 0;
		int bmp= 0;
	    while(!src.eof()){
			char buf[1024];
			src.getline(buf,1024);
			string line= buf;
			cout << line << endl;

			if(bmp){
				if(h){
					parseLineBMP(line,dst);
					h--;
					continue;
				}
				indent.erase(0,2);
				dst << indent << "</" << "BITMAP" << ">" << endl;
				bmp= 0;
				saveBox(dst);
			}

			if(startsWith(line,tagSTARTFONT)){ parseLineSTARTFONT(line,dst); continue;}
			if(startsWith(line,tagENDFONT)){ parseLineENDFONT(line,dst); continue;}
			if(startsWith(line,tagCOMMENT)){ parseLineCOMMENT(line,dst); continue;}
			if(startsWith(line,tagFONT)){ parseLineFONT(line,dst); continue;}
			if(startsWith(line,tagSIZE)){ parseLineSIZE(line,dst); continue;}
			if(startsWith(line,tagFONTBOUNDINGBOX)){ parseLineFONTBOUNDINGBOX(line,dst); continue;}
			if(startsWith(line,tagSTARTPROPERTIES)){ parseLineSTARTPROPERTIES(line,dst); continue;}
			if(startsWith(line,tagENDPROPERTIES)){ parseLineENDPROPERTIES(line,dst); continue;}
			if(startsWith(line,tagCHARS)){ parseLineCHARS(line,dst); continue;}
			if(startsWith(line,tagSTARTCHAR)){ parseLineSTARTCHAR(line,dst); continue;}
			if(startsWith(line,tagENDCHAR)){ parseLineENDCHAR(line,dst); continue;}
			if(startsWith(line,tagENCODING)){ parseLineENCODING(line,dst); continue;}
			if(startsWith(line,tagSWIDTH)){ parseLineSWIDTH(line,dst); continue;}
			if(startsWith(line,tagDWIDTH)){ parseLineDWIDTH(line,dst); continue;}
			if(startsWith(line,tagBBX)){ h= parseLineBBX(line,dst); continue;}
			if(startsWith(line,tagBITMAP)){ parseLineBITMAP(line,dst); bmp= 1; continue;}

			parseLine(line,dst);
		}

	}catch(int err){
		ret= err;
		switch(err){
			case errUsage: 
				cout << "Usage:" << endl;
				cout << "bdf2xml <src.gd> <dst.c>" << endl;
				break;
			case errOpenSrc: 
				cout << "Pbm opening input file " << argv[1] << endl;
				break;
			case errOpenDst: 
				cout << "Pbm opening output file " << argv[2] << endl;
				break;
		}
	}catch(...){
		cout << "Unclassified problem encountered." << endl;
	}
    return ret;
}
