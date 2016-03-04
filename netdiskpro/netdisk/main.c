#include <stdlib.h>
#include <stdio.h>
#include <winsock.h>
#include <mysql.h>
#include <windows.h>
#include <time.h>
#include <io.h>
#include "cgic.h"
#include "ctemplate.h"

MYSQL *mysql;	//全局变量以供使用

void print_error(const char *msg);	//打印出错信息
void printFileUploadMsg(const char *msg);	//打印文件上传消息
void netdisk_action_list();			//响应列出文件消息
void netdisk_action_flush();		//响应文件列表刷新消息
void netdisk_action_add();			//响应上传文件消息
void netdisk_action_del();			//响应文件的删除消息

int cgiMain(int argc, char *argv[])
{
	char action_mod[1024];

//	MessageBoxA(NULL, "debug", "debug", MB_OK);

	mysql = mysql_init(0);	//初始化数据库
	if(!mysql_real_connect(mysql, "127.0.0.1", "root", "root", "manager", 0, 0, 0))
	{
		print_error("Connect error");
		goto exit;
	}
	
	if(mysql_query(mysql, "set names gbk"))
	{
		print_error("Set names error");
		goto exit;
	}

	cgiFormString("action_id", action_mod, sizeof(action_mod));
	if(*action_mod)
	{
		if(!strcmp("file_flush", action_mod))
			netdisk_action_flush();
		else if(!strcmp("file_addnew", action_mod))
			netdisk_action_add();
		else if(!strcmp("file_del", action_mod))
			netdisk_action_del();
			
	}
	else
	{
		netdisk_action_list();
	}

exit:
	mysql_close(mysql);
	return 0;
}

void print_error(const char *msg)
{
	cgiHeaderContentType("text/html;charset=gbk");
	fprintf(cgiOut, "%s : %s", msg, mysql_error(mysql));
}


void printFileUploadMsg(const char *msg)
{
	cgiHeaderContentType("text/html;charset=gbk");
	fprintf(cgiOut, "%s", msg);
}

void netdisk_action_list()
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	TMPL_varlist *varlist = 0;
	TMPL_loop *varloop = 0;

	if(mysql_query(mysql, "select * from m_netdisklist"))
	{
		print_error("Select m_netdisklist error");
		return;
	}

	result = mysql_store_result(mysql);
	while(row = mysql_fetch_row(result))
		varloop = TMPL_add_varlist(varloop, TMPL_add_var(varlist, "file_id", row[0], "file_name", row[1], "file_size", row[2], "file_time", row[3], "file_owener", row[4], 0));
	varlist = TMPL_add_loop(varlist, "files", varloop);
	cgiHeaderContentType("text/html;charset=gbk");
	TMPL_write("netdisk_list.html", 0, 0, varlist, cgiOut, cgiOut);
	mysql_free_result(result);
}

void netdisk_action_flush()
{
	struct _finddata_t find_data;
	long findhandler;
	char sql_commond[1024];
	char file_size[10];
	char file_time[50];

	if(mysql_query(mysql, "delete from m_netdisklist where id>=0"))
	{
		print_error("Delete error");
		return;
	}

	findhandler = _findfirst("netdisk/*.*", &find_data);
	if(-1 == findhandler)
	{
		cgiHeaderContentType("text/html;charset=gbk");
		fprintf(cgiOut, "No such direction");
	}
	
	while(!_findnext(findhandler, &find_data))
	{
		if((find_data.attrib & _A_SUBDIR) == _A_SUBDIR)
		{
			;
		}
		if(find_data.size < 1024)
		{
			sprintf(file_size, "%dB", find_data.size);
		}
		else if(find_data.size < 1024*1024)
		{
			sprintf(file_size, "%0.2fK", find_data.size / 1024.0);
		}
		else if(find_data.size < 1024*1024*1024)
		{
			sprintf(file_size, "%0.2fM", find_data.size / 1024 / 1024.0);
		}
		else
		{
			sprintf(file_size, "%0.2fG", find_data.size / 1024 / 1024 / 1024.0);
		}

		strftime(file_time, sizeof(file_time), "%Y-%m-%d %H-%M-%S", localtime(&find_data.time_create));
		sprintf(sql_commond, "insert into m_netdisklist(f_name,f_size,f_time) values('%s','%s','%s')", find_data.name, file_size, file_time);
		if(mysql_query(mysql, sql_commond))
		{
			print_error("Delete error");
		}
		
	}
	cgiHeaderLocation("netdisk.cgi");
}

void netdisk_action_add()
{
	char uploadfilename[256] = {0};
	char uploadfilepatch[1024];
	char sql_commond[1024];
	char file_size[20];

	if(cgiFormFileName("file_upload", uploadfilename, sizeof(uploadfilename)) == cgiFormSuccess)
	{
		int filesize;
		char filext[10] = {0};

		cgiFormFileSize("file_upload", &filesize);
/*
		if(filesize > 1024*1024*10)
		{
			printFileUploadMsg("文件超过大小！");
			return;
		}
*/
		if(filesize < 1024)
		{
			sprintf(file_size, "%dB", filesize);
		}
		else if(filesize < 1024*1024)
		{
			sprintf(file_size, "%0.2fK", filesize / 1024.0);
		}
		else if(filesize < 1024*1024*1024)
		{
			sprintf(file_size, "%0.2fM", filesize / 1024 / 1024.0);
		}
		else
		{
			sprintf(file_size, "%0.2fG", filesize / 1024 / 1024 / 1024.0);
		}
		_splitpath(uploadfilename, NULL, NULL, NULL, filext);
		if(!stricmp(filext, ".exe"))
		{
			printFileUploadMsg("不能上传后缀为.exe的文件！");
			return;
		}

		{
			cgiFilePtr cgifp;
			char buffer[1024];
			int gotP;
			FILE *localfile;

			sprintf(uploadfilepatch, "netdisk/%s", uploadfilename);
			localfile = fopen(uploadfilepatch, "wb");
			cgiFormFileOpen("file_upload", &cgifp);

			while(cgiFormFileRead(cgifp, buffer, sizeof(buffer), &gotP) == cgiFormSuccess)
			{
				fwrite(buffer, gotP, 1, localfile);
			}

			fclose(localfile);
			cgiFormFileClose(cgifp);
			sprintf(sql_commond, "insert into m_netdisklist(f_name,f_size,f_time) values('%s','%s',now())", uploadfilename, file_size);
			if(mysql_query(mysql, sql_commond))
			{
				print_error("Insert into error");
			}
			printFileUploadMsg("文件上传成功！");
//			Sleep(3);
//			cgiHeaderLocation("netdisk.cgi");
		}
	}
	else
	{
		printFileUploadMsg("没有选择文件！");
		return;
	}
}

void netdisk_action_del()
{
	int id_mod = 0;
	char sql_commond[1024] = {0};
	char mod_del_name[1024];
	char file_del_name[1024];

	cgiFormInteger("id", &id_mod, 0);
	sprintf(sql_commond, "delete from m_netdisklist where id=%d", id_mod);
	if(mysql_query(mysql, sql_commond))
	{
		print_error("Delete error");
	}

	cgiFormString("del_name", mod_del_name, sizeof(mod_del_name));
	sprintf(file_del_name, "netdisk/%s", mod_del_name);
	remove(file_del_name);

	cgiHeaderLocation("netdisk.cgi");
}