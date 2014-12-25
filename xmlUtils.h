/* 
 * File:   xmlUtils.h
 * Author: nahit7
 *
 * Created on November 29, 2014, 12:35 PM
 */
#include <libxml/parser.h>
#include <libxml/xpath.h>


    
xmlDocPtr getdoc(char *docname);
xmlXPathObjectPtr getnodeset(xmlDocPtr doc, xmlChar *xpath);
int numChilds(char* fusePath,xmlDocPtr doc);
char **readDir(char* fusePath,xmlDocPtr doc,int fstatus[]);




