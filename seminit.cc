// This file is part of cobcy, a COBOL-to-C compiler.
//
// Copyright (c) 1995-2008 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "semextern.h"
#include "semcontrol.h"
#include "semdecl.h"
#include "semfile.h"
#include "symfile.h"
#include "symvar.h"

//----------------------------------------------------------------------

ostringstream 		codef;
ostringstream 		declf;
vector<StackEntry> 	g_Tokens;
int 			NestingLevel = 0;
bool			CodeBegan = false;

//----------------------------------------------------------------------

void FinishDecl (void)
{
    DTRACE ("DBG: Finishing declarations\n");
    CloseScopeLevels (0);
    AssociateRecordsWithFD();
    codef << "\n";
}

void StartCode (void)
{
    FinishDecl();
    DTRACE ("DBG: Starting code, initializing variables\n");
    InitializeVariables();

    DTRACE ("DBG: Beginning first paragraph\n");
    GenIndent(); codef << "static int _FirstParagraph (void)\n{\n";
    ++ NestingLevel;
    CodeBegan = true;
}

void EndCode (void)
{
    DTRACE ("DBG: Finished code, genrating main()\n");
    codef << "int main (void)\n{\n";
    ++ NestingLevel;

    GenIndent(); codef << "_SetVarValues();\n";
    OpenSpecialFiles();		// Generates code if there are any
    GenParagraphCalls();
    CloseSpecialFiles();
    GenIndent(); codef << "return EXIT_SUCCESS;\n}\n";
    -- NestingLevel;
}

void StartProgram (void)
{
    string sourceName (g_Config.CodeFile);
    auto slashpos = sourceName.rfind ('/');
    if (slashpos != string::npos)
	sourceName.erase (0, slashpos+1);

    string cobolName (g_Config.SourceFile);
    slashpos = cobolName.rfind ('/');
    if (slashpos != string::npos)
	cobolName.erase (0, slashpos+1);

    declf << "// " << sourceName << "\n";
    declf << "// Generated by Cobol-to-C compiler from " << cobolName << ".\n\n";
    declf << "#include <coblib/cobfunc.h>\n\n";	// For internal functions
    assert (g_Symbols.empty());

    // Space filler variable
    DTRACE ("DBG: Declaring _space_var\n");
    auto spacevar = g_Symbols.emplace<CobolVar> ("_space_var");
    spacevar->SetPicture ("x(200)");
    spacevar->SetName ("_space_var");

    // Zero filler variable
    DTRACE ("DBG: Declaring _zero_var\n");
    auto zerovar = g_Symbols.emplace<CobolVar> ("_zero_var");
    zerovar->SetPicture ("9(1)");
    zerovar->SetName ("_zero_var");

    // Default output stream
    DTRACE ("DBG: Declaring stdout\n");
    auto defoutf = g_Symbols.emplace<CobolFile> ("stdout");
    defoutf->SetName ("stdout");
    defoutf->SetOrganization (ORG_Sequential);
    defoutf->SetAccessMode (AM_Sequential);
}

void EndProgram (void)
{
    if (ErrorOccured())
	return;
    string outfile = declf.str();
    outfile += codef.str();
    outfile.write_file (g_Config.CodeFile);
}
