#include "all.h"

INT8  g_szCfgFilePath[MAX_FILE_PATH_LEN];

void * GET_MEM(INT32 size)
{
	if ( size <= 0 )  
		return NULL;  
	else		
		return malloc(size);
}			    
							
void RET_MEM(void *pData)
{
	if (pData == NULL) 
		return; 
	else 
		free(pData); 
	return; 
}

void TrimLeft(INT8 *s)
{
    INT32 i=0, j=0;
    if(!strlen(s)) 
		return;

    while( s[i] == ' '&& s[i] != '\0')
		i++;

    while( s[i] != '\0')
		s[j++] = s[i++];
    s[j] = '\0';
}

void TrimRight(INT8 *s)
{
    INT32 pos;
    pos = strlen(s) - 1;
    while( s[pos] == ' ' ) {
        s[pos--] = '\0';
        if(pos<0) break;
    }
}

void Trim(INT8 *s)
{
    TrimLeft(s);
    TrimRight(s);
}


void GetIniKeyString(INT8 *szTitle, INT8 *szKey, INT8 *szBuf, INT32 BufLen, INT8 *szFilePath )   
{   
    FILE *fp;   
    INT8 szLine[1024];  
    static INT8 tmpstr[1024];  
    INT32 rtnval;  
    INT32 i = 0;   
    INT32 flag = 0;   
    INT8 *tmp;  
  
    if((fp = fopen(szFilePath, "r")) == NULL)   
    {   
        return;  
    }  
    while(!feof(fp))   
    {
		rtnval = fgetc(fp);   
		if(rtnval == EOF)   
		{
			break;
		}
        else   
        {   
			szLine[i++] = rtnval;   
        }   
        if(rtnval == '\n')   
        {   

            //i--;  
  
            szLine[--i] = '\0';  
            i = 0;   
            tmp = strchr(szLine, '=');   
  
            if(( tmp != NULL )&&(flag == 1))   
            {   
                if(strstr(szLine, szKey)!=NULL)   
                {   
                    //注释行   
                    if ('#' == szLine[0])  
                    {  
                    }  
                    else if ( '/' == szLine[0] && '/' == szLine[1] )  
                    {  

                    }  
                    else  
                    {  
                        //找打key对应变量   
                        Trim(tmp+1);
						snprintf(szBuf, BufLen, "%s", tmp+1 );
                        fclose(fp);  
                        return;   
                    }  
                }   
            }  
            else   
            {   
                strcpy(tmpstr,"[");   
                strcat(tmpstr,szTitle);   
                strcat(tmpstr,"]");  
                if( strncmp(tmpstr,szLine,strlen(tmpstr)) == 0 )   
                {  
                    flag = 1;   
                }  
            }  
        }  
    }  
    fclose(fp);    
}  

INT32 GetIniKeyInt(INT8 *szTitle, INT8 *szKey, INT8 *szFilePath)  
{   
	INT8 szBuf[1024] = {0};
	
    GetIniKeyString( szTitle,  szKey, szBuf, 1024, szFilePath); 
	return atoi(szBuf);
}  



INT32 InitCfgPath()
{
	INT8 *pHome = NULL;

	pHome = getenv("HOME");
	if ( pHome == NULL )
	{
		return RESULT_OPER_SYS_ERR;
	}

	snprintf( g_szCfgFilePath, 1024, "%s%s%s", pHome, "/etc/", CONF_FILE_PATH );
	return RESULT_OK;
}
