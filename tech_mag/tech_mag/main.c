#include <stdlib.h>
#include <stdio.h>
#include <winsock.h>
#include <mysql.h>
#include <windows.h>
#include "cgic.h"
#include "ctemplate.h"

void ExecSelectQuery(char *sql, char *commond, int option);
MYSQL_RES* sql_query_return(MYSQL *mysql, char *commond);	//查询数据库并返回结果
MYSQL *mysql;	//全局变量，为方便以后函数的编写分块

void action_teacher_list_request();
void action_teacher_del_request();
void action_addnew();
void action_addnew_request();
void action_edit_request();
void action_edit();
void print_select(char *buffer, char *sql, char *select_option);

int cgiMain(int argc, char *argv[])
{
	char action_id_name[255];	//动作的id名称，包括删除，添加等等
	
	mysql = mysql_init(0);

//	MessageBoxA(NULL, "debug", "ddebug", MB_OK);

	/* 初始化数据库，进行数据库的连接 */
	if(!mysql_real_connect(mysql, "localhost", "root", "root", "manager", 0, 0, 0))
	{
		printf("Can't connect study!\nerror: %s\n", mysql_error(mysql));
		goto exit;
	}
	if(mysql_query(mysql, "set names gbk"))
	{
		printf("Set name error: %s", mysql_error(mysql));
		goto exit;
	}

	/* cgi程序主体 */
	cgiFormString("action_id", action_id_name, sizeof(action_id_name));	//取得返回的action_id的值

	if(*action_id_name)
	{
		if(!strcmp(action_id_name, "action_addnew"))
			action_addnew();
		else if(!strcmp(action_id_name, "request_addnew"))
			action_addnew_request();
		else if(!strcmp(action_id_name, "teacher_list"))
			cgiHeaderLocation("tech_mag.cgi");
		else if(!strcmp(action_id_name, "teacher_del"))
			action_teacher_del_request();
		else if(!strcmp(action_id_name, "request_edit"))
			action_edit_request();
		else if(!strcmp(action_id_name, "action_edit"))
			action_edit();
	}
	else
		action_teacher_list_request();
	
exit:
	mysql_close(mysql);
	return 0;
}

MYSQL_RES* sql_query_return(MYSQL *mysql, char *commond)
{
	if(*commond)
	{
		if(mysql_query(mysql, commond))
		{
			printf("Exectue error: %s", mysql_error(mysql));
			goto exit;
		}
		{
			MYSQL_RES *result = mysql_store_result(mysql);
			return result;
		}
	}

exit:
	mysql_close(mysql);
	return NULL;
}

void ExecSelectQuery(char *sql, char *commond, int option)
{
	MYSQL *mysql = mysql_init(0);

	if(!mysql_real_connect(mysql, "localhost", "root", "root", sql, 0, 0, 0))
	{
		printf("Can't connect study!\nerror: %s\n", mysql_error(mysql));
		goto exit;
	}
	if(mysql_query(mysql, "set names gbk"))
	{
		printf("Set name error: %s", mysql_error(mysql));
		goto exit;
	}

	if(*commond)
	{
		if(mysql_query(mysql, commond))
		{
			printf("Exectue error: %s", mysql_error(mysql));
			goto exit;
		}
	}

	if(option)
	{
		if(mysql_query(mysql, "select * from M_teacherlist"))
		{
			printf("Select error: %s", mysql_error(mysql));
			goto exit;
		}

		{
			MYSQL_RES *result = mysql_store_result(mysql);
			MYSQL_ROW row;

			while(row = mysql_fetch_row(result))
			{
				printf("Id = %s\tTeacherName = %s\tGender = %s\tphone = %s\t\n",row[0], row[1], row[2], row[3]);
			}
			mysql_free_result(result);
			getchar();
		}
	}

exit:
	mysql_close(mysql);
}

void action_addnew()
{
	char t_name[256] = {0};
	char t_gender[10] = {0};
	char t_phone[20] = {0};
	char sql_commond[1024] = {0};
	int i;

	cgiFormString("Tadd_name", t_name, sizeof(t_name));
	cgiFormString("tgender_select", t_gender, sizeof(t_gender));
	cgiFormString("Tadd_phone", t_phone, sizeof(t_phone));
	for(i = 0; i < 15 & (t_phone[i] != '\0'); i ++)
	{
		if(t_phone[i] >= '0' & t_phone[i] <= '9')
			continue;
		else
		{
			TMPL_varlist *varlist = 0;
			varlist = TMPL_add_var(varlist, "mod_sel", "教师新增", "mod", "action_addnew", "error", "请填写正确的号码格式",0);
			cgiHeaderContentType("text/html;charset=gbk");
			TMPL_write("teacher_addnew.html", 0, 0, varlist, cgiOut, cgiOut);

			return;
		}
	}
	if(i > 11)
	{
		TMPL_varlist *varlist = 0;
		varlist = TMPL_add_var(varlist, "mod_sel", "教师新增", "mod", "action_addnew", "error", "号码超过长度",0);
		cgiHeaderContentType("text/html;charset=gbk");
		TMPL_write("teacher_addnew.html", 0, 0, varlist, cgiOut, cgiOut);

		return;
	}
	sprintf(sql_commond, "insert into M_teacherlist(name,gender,phone) values('%s','%s','%s')", t_name, t_gender, t_phone);

	ExecSelectQuery("manager", sql_commond, 0);
	cgiHeaderLocation("tech_mag.cgi?action_id=teacher_list");
}

void action_addnew_request()
{
	TMPL_varlist *varlist = 0;
	char buffer_gender[1024] = {0};

	print_select(buffer_gender, "select * from ms_genders", "5");

	varlist = TMPL_add_var(varlist, "mod_sel", "教师新增", "mod", "action_addnew", "tgender_option", buffer_gender, 0);
	cgiHeaderContentType("text/html;charset=gbk");
	TMPL_write("teacher_addnew.html", 0, 0, varlist, cgiOut, cgiOut);
}

void action_teacher_del_request()
{
	int id_mod = 0;
	char sql_commond[1024] = {0};

	cgiFormInteger("id", &id_mod, 0);
	sprintf(sql_commond, "delete from M_teacherlist where id=%d", id_mod);
	ExecSelectQuery("manager", sql_commond, 0);

	cgiHeaderLocation("tech_mag.cgi?action_id=teacher_list");
}

void action_teacher_list_request()
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	TMPL_varlist *varlist = 0;
	TMPL_loop *varloop = 0;

	if(mysql_query(mysql, "select M_teacherlist.id,M_teacherlist.name,ms_genders.genders,M_teacherlist.phone from M_teacherlist left join ms_genders on m_teacherlist.gender=ms_genders.id"))
	{
		printf("Select error: %s", mysql_error(mysql));
		mysql_close(mysql);
	}

	result =  mysql_store_result(mysql);

	while(row = mysql_fetch_row(result))
		varloop = TMPL_add_varlist(varloop, TMPL_add_var(varlist, "ID", row[0], "teacher_name", row[1], "teacher_gender", row[2], "teacher_phone", row[3], 0));

	varlist = TMPL_add_loop(varlist, "teachers", varloop); 
	cgiHeaderContentType("text/html;charset=gbk");
	TMPL_write("teacher_list.html", 0, 0, varlist, cgiOut, cgiOut);
	mysql_free_result(result);
}

void action_edit_request()
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	TMPL_varlist *varlist = 0;
	int id_mod = 0;
	char id_str[10];
	char sql_commond[1024] = {0};
	char gender_mod[256] = {0};

	cgiFormInteger("id", &id_mod, 0);
	sprintf(sql_commond, "select M_teacherlist.name,ms_genders.genders,M_teacherlist.phone,M_teacherlist.gender from M_teacherlist left join ms_genders on m_teacherlist.gender=ms_genders.id where M_teacherlist.id=%d", id_mod);

	if(mysql_query(mysql, sql_commond))
	{
		printf("Select error: %s", mysql_error(mysql));
		mysql_close(mysql);
	}

	result =  mysql_store_result(mysql);
	itoa(id_mod, id_str, 10);

	if(row = mysql_fetch_row(result))
	{
		print_select(gender_mod, "select * from ms_genders", row[3]);
		varlist = TMPL_add_var(varlist, "mod_sel", "教师编辑", "mod", "action_edit","edit_id", id_str, "t_name", row[0],"t_gender", row[1], "t_phone", row[2], "tgender_option", gender_mod, 0);
	}
	else
	{
		printf("Select error: %s", mysql_error(mysql));
		mysql_close(mysql);
	}

	mysql_free_result(result);

	cgiHeaderContentType("text/html;charset=gbk");
	TMPL_write("teacher_addnew.html", 0, 0, varlist, cgiOut, cgiOut);
}

void action_edit()
{
	char t_name[256] = {0};
	char t_gender[10] = {0};
	char t_phone[20] = {0};
	char sql_commond[1024] = {0};
	int id_mod;
	char id_str[10];

	cgiFormString("edit_id", id_str, sizeof(id_str));
	id_mod = atoi(id_str);

	cgiFormString("Tadd_name", t_name, sizeof(t_name));
	cgiFormString("tgender_select", t_gender, sizeof(t_gender));
	cgiFormString("Tadd_phone", t_phone, sizeof(t_phone));
	sprintf(sql_commond, "update M_teacherlist set name='%s',gender='%s',phone='%s' where id=%d", t_name, t_gender, t_phone, id_mod);

	ExecSelectQuery("manager", sql_commond, 0);
	cgiHeaderLocation("tech_mag.cgi?action_id=teacher_list");
}

void print_select(char *buffer, char *sql, char *select_option)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	int len = 0;

	if(mysql_query(mysql, sql))
	{
		printf("Select error: %s", mysql_error(mysql));
		mysql_close(mysql);
		return;
	}
	result = mysql_store_result(mysql);
	
	while(row = mysql_fetch_row(result))
	{
			if(strcmp(select_option, row[0]) == 0)
			{
				len = sprintf(buffer, "<option value='%s' selected='selected'>%s</option>", row[0], row[1]);
			}
			else
			{
				len = sprintf(buffer, "<option value='%s'>%s</option>", row[0], row[1]);
			}
			buffer += len;
	}

	mysql_free_result(result);
}