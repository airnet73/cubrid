/*
 * Copyright 2008 Search Solution Corporation
 * Copyright 2016 CUBRID Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
// ManageRegistry.cpp: implementation of the CManageRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cubridtray.h"
#include "ManageRegistry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define	REGKEY_ROOT					"SOFTWARE\\CUBRID"
#define	REGKEY_PROD_ROOT			"RootPath"
#define REGKEY_JAVA					"Software\\JavaSoft\\Java Runtime Environment"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CManageRegistry::CManageRegistry( char* sprodname )
{
	hKeyReg     = NULL;
	sResultName = NULL;

	if( sprodname )
	{
		memset( sKeyPath, 0x00, sizeof( sKeyPath ) );
		sprintf( sKeyPath, "%s\\%s", REGKEY_ROOT, sprodname );
		bOpenRegistry();
	}
}

CManageRegistry::~CManageRegistry()
{
	if( hKeyReg  ) RegCloseKey( hKeyReg );
	if( sResultName ) delete sResultName;

	hKeyReg  = NULL;
	sResultName = NULL;
}



bool CManageRegistry::bOpenRegistry()
{
	DWORD   rc;

	if( !sKeyPath )
		return false;

	rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
						sKeyPath,
						0,
						KEY_QUERY_VALUE,
						&hKeyReg );

	if (rc != ERROR_SUCCESS) {
		return false;
	}

	return true;
}



char* CManageRegistry::sGetItem( char* sItemName )
{
#define BUF_SIZE 2048

	char* sValue;
	char sBuff[BUF_SIZE];

	DWORD   rc;
	DWORD   len = BUF_SIZE;
	DWORD   dwType;

	if(!sItemName || !hKeyReg || !sKeyPath) return NULL;

	memset(sBuff, 0x00, BUF_SIZE);

	rc = RegQueryValueEx( hKeyReg, sItemName, 0, &dwType, (LPBYTE)sBuff, &len );

	if (rc != ERROR_SUCCESS)
	  {
		return NULL;
	  }

	if(!sBuff || len <= 0)
	  {
		return NULL;
	  }

	sValue = new char[len + 1];

	strncpy( sValue, sBuff, len );

	return sValue;
}




char* CManageRegistry::sGetString( char* sItemName, char* sCmdName )
{
	char* sPath = sGetItem( sItemName );

	if( !sPath ) return NULL;

	int dSize = strlen( sPath ) + strlen( sCmdName );
	char* sFullName = new char[ dSize + 5 ];
	memset( sFullName, 0x00, dSize + 5 );
	sprintf( sFullName, "%s\\%s", sPath, sCmdName );

	return sFullName;
}



int CManageRegistry::dGetString( char* sItemName, char* sCmdName )
{
	char* sPath = sGetItem( sItemName );

	if( !sPath ) return 0;

	int dSize = strlen( sPath ) + strlen( sCmdName );
	sResultName = new char[ dSize + 5 ];
	memset( sResultName, 0x00, dSize + 5 );
	sprintf( sResultName, "%s\\%s", sPath, sCmdName );

	return strlen( sResultName );
}


bool CManageRegistry::bGetString( char* sString )
{
	strcpy( sString, sResultName );
	return true;
}



bool CManageRegistry::GetJavaRootPath(char *path)
{
	DWORD   rc;
	DWORD   len;
	DWORD   dwType;
	short   reg_mask = 0;

	char currentVersion[16];
	char regkey_java_current_version[2048];
	char java_root_path[2048];

	if (!path) return false;

	reg_mask = KEY_QUERY_VALUE;

	rc = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
						REGKEY_JAVA,
						0,
						reg_mask,
						&hKeyReg );

	if (rc != ERROR_SUCCESS) return false;

	len =  sizeof(currentVersion);
	rc = RegQueryValueEx( hKeyReg, "CurrentVersion", 0, &dwType, (LPBYTE)currentVersion, &len );

	if( hKeyReg  ) RegCloseKey( hKeyReg );

	if (rc != ERROR_SUCCESS) return false;

	hKeyReg = NULL;

	sprintf(regkey_java_current_version, "%s\\%s", REGKEY_JAVA, currentVersion);
	rc = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
						regkey_java_current_version,
						0,
						reg_mask,
						&hKeyReg );

	if (rc != ERROR_SUCCESS) return false;
	len = sizeof(java_root_path);
	rc = RegQueryValueEx( hKeyReg, "JavaHome", 0, &dwType, (LPBYTE)java_root_path, &len );

	if (rc != ERROR_SUCCESS) return false;

	strcpy(path, java_root_path);

	return true;
}
