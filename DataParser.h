/*
* Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef LIBUSEFUL_DATAPARSE_H
#define LIBUSEFUL_DATAPARSE_H

#include "includes.h"

/*
DataParser functions provide a method for parsing JSON, RSS, and YAML documents and also 'ini file' and 'config file' documents
as described below.

'ParserParseDocument' is passed the document type and document text (which can be read into a
string with STREAMReadDocument) and returns a ListNode * that points to the root of a parse tree.

Values of 'DocType' recognized by ParserParseDocument are: json, rss, yaml, config

Items in the parse tree are represented by ListNode items (see List.h). ListNode structures map to parser items
like so:

typedef struct
{
char *Tag;      //name of parse item
int ItemType;   //type of parse item
void *Item;     //contents of parse item
} ListNode;


Parse items can be 'values', in which case the 'Item' member of list node points to a string that holds their
value, or 'Entities' and 'Arrays', in which case the 'Item' member points to a child list of items. For instance
consider the example:

const char *Document="key1=value1\nkey2=value2\nkey3=value3\n";
ListNode *Tree, *Curr;

Tree=ParserParseDocument("config", Document);
Curr=ListGetNext(Tree);
while (Curr)
{
printf("%s = %s\n",Curr->Tag, (char *) Curr->Item);
Curr=ListGetNext(Tree);
}


This should print out item names and values. Now consider a document like:

Thing
{
attribute1=value1
attribute2=value2
attribute3=value3
}


Out previous code would now only find one item in the list, and if it tired to print the value of that item, it
would likely crash! It should check the item type:

Tree=ParserParseDocument("config", Document);
Curr=ListGetNext(Tree);
while (Curr)
{
if (ParseItemIsValue(Curr)) printf("atrrib: %s = %s\n",Curr->Tag, (char *) Curr->Item);
else printf("entity: %s\n",Curr->Tag);
Curr=ListGetNext(Tree);
}

But now it will only display one output, 'entity: Thing'. The members of thing can be accessed by:

SubList=(ListNode *) Curr->Item;

and then using 'ListGetNext' on members of the SubList, but this can get pretty messy when entities contain
other entitites, so the following functions are used to simplify access:

ListNode *ParserOpenItem(ListNode *Items, const char *Name);

ParserOpenItem returns the members of an entitiy. Entities within entities are represented in a manner similar to
directory paths. e.g.

Root=ParserParseDocument("json", Document);
Student=ParserOpenItem(Root, "/School/Class/Students/BillBlogs");
Curr=ListGetNext(Student);
while (Curr)
{
if (Curr->ItemType==ITEM_VALUE) printf("%s = %s\n",Curr->Tag, (char *) Curr->Item);
Curr=ListGetNext(Tree);
}

const char *ParserGetValue(ListNode *Items, const char *Name);

ParserGetValue finds an item of a given name, and returns its value. The 'ItemType' is checked, and NULL is
returned if the item isn't an ITEM_VALUE type. e.g.

Root=ParserParseDocument("json", Document);
Student=ParserOpenItem(Root, "/School/Class/Students/BillBlogs");
printf("Grade Average: %s\n",ParserGetValue(Student, "grade_avg"));
printf("Days Sick: %s\n",ParserGetValue(Student, "sickdays"));


When finished you should free the top level tree by passing it to 'ParserItemsDestroy'. NEVER DO THIS WITH BRANCHES
OBTAINED VIA ParserOpenItem. Only do it to the root node obtained with ParserParseDocument

'config' style documents contain either

<name> <value>
<name>:<value>
<name>=<value>

<group name>
{
<name>=<value>
<name>=<value>
}

so lines consist of a name, and a value seperated by space, colon or '='
names can be quoted if they contain spaces, so:

"ultimate answer" = 42

values do not need to be quoted in the same way, as they are read until the end of line
the value is treated as a string *including any quotes it contains*. Thus you can use
quotes within values and have those passed to the program. e.g.:

command="tar -zcf /tmp/myfile.tgz /root" user=root group=backup

will set the value to be the entire string, including any quotes

key-value pairs can be grouped into objects like so:

"bill blogs"
{
FirstName=Bill
LastName=Blogs
Age=immortal
}





## CMON - Colum's Minimal Object Notation

CMON is an experimental untyped 'lazy' object notation. It's mostly intended as a quick-and-dirty notation to compactly store and load data into programs which know the datatypes in advance. Thus everything is stored as strings and the program does any conversion to 'int' or 'bool' because it knows what data it's using. It's aimed at simple use cases where even typing JSON, and getting all the " and , lined up is too much bother.

CMON's main use cases are 
  1) 'quick and dirty' config files consisting of lines of key-value pairs, 
  2) 'record files', a kind of database made up of single-line entities/objects 
  3) debugging output: CMON is intended to be 'viewable' and has 'collapse' mode when exporting, which tries to write data as succinctly as possible. This is sometimes useful when importing formats like JSON to get an overview of what was imported.

CMON supports comments starting with #:

  # this is a comment

it supports single-line values

  name=value

CMON tries to handle names and values that contain spaces. Everything is treated as a string until one of the characters ={}[];\n is encountered. '=' indicats that preceeding text is the name of a name-value pair. Semi-colon or newline will end the value part of a name-value pair, (but note extra rules for 'single-line objects' below). Both " and ' can be used as quoting characters, so if these occur in string they must be quoted bash-style, using \ or by using the other type of quotes in the outer. e.g.:

  value1="multi word value"
  value2='another multi word value'
  value3=a value that is ended by newline
  value4=two values in one line; value5=via use of semi-colon
  value number 6 = a value that has spaces in the name
  quoted value=this is a value with an apostrophe in it\'s data
  quoted value2="this is also value with an apostrophe in it's data"
  unquoted value=this value will go wrong because it's got an unquoted apostrophe
  
CMON supports 'single line objects' like so:

  my_object: value1=this value2=that value3="the other"

single line objects are a special case where whitespacespace will break members up as shown above. You should likely always quote members of single-line objects. They are the only use-case where whitespace has this effect.

CMON also supports multi-line objects like so:

  my_object {
  value1=this
  value2=that
  }

or like so:

  my_object 
  {
  value1=this
  value2=that
  }


or if you want to use this syntax in a single-line form ';' can be used as an end character, like so:

  my_object { value1=this; value2=that; value3=the other; value4=whatever }

This is because objects created using the {} syntax do not break up their members using whitespace.


You can nest objects

  outer_object {
   inner1: value1=this value2=that
   inner2: value1=something value2=something
   inner3 {
     value1=whatever
     value2=again
   }
   inner_array [this; that; the other]
  }


Arrays have the form:

  myarray [this; that; "the other"]

or

  myarray [
  this
  that
  "the other"
  ]

or

  myarray 
  [
  this
  that
  "the other"
  ]


Arrays cannot contain named items. Note the use of semi-colon to divide up array items. This was chosen as comma occurs too often in text to be used for this purpose without quoting commas.



CMON is parsed similarly to any other format: 
  Root=ParserParseDocument("cmon", Document);
  Student=ParserOpenItem(Root, "/School/Class/Students/BillBlogs");
  printf("Grade Average: %s\n",ParserGetValue(Student, "grade_avg"));
  printf("Days Sick: %s\n",ParserGetValue(Student, "sickdays"));


When exporting CMON there's three possible 'export modes',

Tempstr=ParserExport(Tempstr, "cmon", P);

Tempstr=ParserExport(Tempstr, "cmon+expand", P);

Tempstr=ParserExport(Tempstr, "cmon+collapse", P);

These modes control how CMON expands or collapses arrays and objects. "cmon+expand" will always produce output where arrays and objects are multi-line. "cmon+collapse" will, where possible, produce output with 'one line' objects and arrays. Just plain 'cmon' will try to produce a sensible mix of these two.
*/



#ifdef __cplusplus
extern "C" {
#endif

typedef enum {PARSER_JSON, PARSER_XML, PARSER_RSS, PARSER_YAML, PARSER_CONFIG, PARSER_INI, PARSER_URL, PARSER_CMON} EParsers;

//this typedef is simply to create a typename that makes code clearer, you can just use 'ListNode' if you prefer
typedef struct lnode PARSER;

#define ITEM_ROOT    0          //dummy header at top level of the parse tree
#define ITEM_STRING  1          //text string
#define ITEM_ENTITY  2          //an object/set with members
#define ITEM_ARRAY   3          //a list of things
#define ITEM_INTEGER 4          //integer value (actually number rather than integer)
#define ITEM_INTERNAL_LIST 100  //list of items that is internal to an entity or array
#define ITEM_ENTITY_LINE   102  //an entity that should be expressed as a single line when output
#define ITEM_ARRAY_LINE    103  //array that syhould be expressd as a single line when output


ListNode *ParserParseDocument(const char *DocType, const char *Doc);
void ParserItemsDestroy(ListNode *Items);
ListNode *ParserFindItem(ListNode *Items, const char *Name);
ListNode *ParserSubItems(ListNode *Node);
ListNode *ParserOpenItem(ListNode *Items, const char *Name);
int ParserItemIsValue(ListNode *Node);
const char *ParserGetValue(ListNode *Items, const char *Name);
ListNode *ParserAddValue(ListNode *Parent, const char *Name, const char *Value);
ListNode *ParserNewObject(ListNode *Parent, int Type, const char *Name);
ListNode *ParserAddArray(ListNode *Parent, const char *Name);
char *ParserExport(char *RetStr, const char *Format, PARSER *P);

#ifdef __cplusplus
}
#endif


#endif
