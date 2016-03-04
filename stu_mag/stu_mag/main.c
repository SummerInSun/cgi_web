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

void action_student_list_request();
void action_student_del_request();
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
		else if(!strcmp(action_id_name, "student_list"))
			cgiHeaderLocation("stu_mag.cgi");
		else if(!strcmp(action_id_name, "student_del"))
			action_student_del_request();
		else if(!strcmp(action_id_name, "request_edit"))
			action_edit_request();
		else if(!strcmp(action_id_name, "action_edit"))
			action_edit();
	}
	else
		action_student_list_request();
	
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
		if(mysql_query(mysql, "select * from M_studentlist"))
		{
			printf("Select error: %s", mysql_error(mysql));
			goto exit;
		}

		{
			MYSQL_RES *result = mysql_store_result(mysql);
			MYSQL_ROW row;

			while(row = mysql_fetch_row(result))
			{
				printf("Id = %s\tstudentName = %s\tGender = %s\tphone = %s\t\n",row[0], row[1], row[2], row[3]);
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
	char s_name[256] = {0};
	char s_date[20] = {0};
	char s_height[20] = {0};
	char s_weight[20] = {0};
	char s_gender[10] = {0};
	char s_area[256] = {0};
	char s_race[20] = {0};
	char sql_commond[1024] = {0};
	int i;
	     

	cgiFormString("Sadd_name", s_name, sizeof(s_name));
	cgiFormString("Sadd_date", s_date, sizeof(s_date));
	cgiFormString("Sadd_height", s_height, sizeof(s_height));
	cgiFormString("Sadd_weight", s_weight, sizeof(s_weight));
	cgiFormString("gender_select", s_gender, sizeof(s_gender));
	cgiFormString("race_select", s_race, sizeof(s_race));
	cgiFormString("Sadd_area", s_area, sizeof(s_area));

	sprintf(sql_commond, "insert into M_studentlist(name,genderID,raceID,height,weight,area,date) values('%s','%d','%d','%s','%s','%s','%s')", s_name, atoi(s_gender), atoi(s_race), s_height, s_weight, s_area, s_date);

	ExecSelectQuery("manager", sql_commond, 0);
	cgiHeaderLocation("stu_mag.cgi?action_id=student_list");
}

void action_addnew_request()
{
	TMPL_varlist *varlist = 0;
	char buffer_gender[1024] = {0};
	char buffer_race[1024] = {0};

	print_select(buffer_gender, "select * from ms_genders", "5");
	print_select(buffer_race, "select * from ms_races", "5");

	varlist = TMPL_add_var(varlist, "mod_sel", "学生新增", "mod", "action_addnew", "gender_option", buffer_gender, "race_option", buffer_race, 0);
	cgiHeaderContentType("text/html;charset=gbk");
	TMPL_write("student_addnew.html", 0, 0, varlist, cgiOut, cgiOut);

}

void action_student_del_request()
{
	int id_mod = 0;
	char sql_commond[1024] = {0};

	cgiFormInteger("id", &id_mod, 0);
	sprintf(sql_commond, "delete from M_studentlist where id=%d", id_mod);
	ExecSelectQuery("manager", sql_commond, 0);

	cgiHeaderLocation("stu_mag.cgi?action_id=student_list");
}

void action_student_list_request()
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	TMPL_varlist *varlist = 0;
	TMPL_loop *varloop = 0;

	if(mysql_query(mysql, "select m_studentlist.id,m_studentlist.name,ms_genders.genders,ms_races.races,m_studentlist.height,m_studentlist.weight,m_studentlist.area,m_studentlist.date from m_studentlist left join ms_genders on m_studentlist.genderid=ms_genders.id left join ms_races on m_studentlist.raceid=ms_races.id"))
	{
		printf("Select error: %s", mysql_error(mysql));
		mysql_close(mysql);
		return;
	}

	result =  mysql_store_result(mysql);

	while(row = mysql_fetch_row(result))
		varloop = TMPL_add_varlist(varloop, TMPL_add_var(varlist, "ID", row[0], "student_name", row[1], "student_gender", row[2], "student_race", row[3], "student_height",row[4], "student_weight", row[5], "student_area", row[6], "student_date", row[7], 0));

	varlist = TMPL_add_loop(varlist, "students", varloop); 
	cgiHeaderContentType("text/html;charset=gbk");
	TMPL_write("student_list.html", 0, 0, varlist, cgiOut, cgiOut);
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
	char race_mod[1024] = {0};
	char gender_mod[1024] = {0};

	cgiFormInteger("id", &id_mod, 0);
	sprintf(sql_commond, "select m_studentlist.name,ms_genders.genders,ms_races.races,m_studentlist.height,m_studentlist.weight,m_studentlist.area,m_studentlist.date,m_studentlist.genderid,m_studentlist.raceid from m_studentlist left join ms_genders on m_studentlist.genderid=ms_genders.id left join ms_races on m_studentlist.raceid=ms_races.id where M_studentlist.id=%d", id_mod);

	if(mysql_query(mysql, sql_commond))
	{
		printf("Select error: %s", mysql_error(mysql));
		mysql_close(mysql);
		return;
	}

	result =  mysql_store_result(mysql);
	itoa(id_mod, id_str, 10);

	if(row = mysql_fetch_row(result))
	{
		print_select(race_mod, "select * from ms_races", row[8]);
		print_select(gender_mod, "select * from ms_genders", row[7]);
		varlist = TMPL_add_var(varlist, "s_name", row[0], "gender_option", gender_mod, "race_option", race_mod, "s_height", row[3], "s_weight",row[4], "s_area", row[5], "s_date", row[6], "mod_sel", "学生编辑", "edit_id", id_str, "mod", "action_edit", 0);
	}
	else
	{
		printf("Select error: %s", mysql_error(mysql));
		mysql_close(mysql);
		return;
	}

	mysql_free_result(result);

	cgiHeaderContentType("text/html;charset=gbk");
	TMPL_write("student_addnew.html", 0, 0, varlist, cgiOut, cgiOut);
}

void action_edit()
{
	char s_name[256] = {0};
	char s_date[20] = {0};
	char s_height[20] = {0};
	char s_weight[20] = {0};
	char s_gender[10] = {0};
	char s_area[256] = {0};
	char s_race[20] = {0};
	char sql_commond[1024] = {0};
	int id_mod;     
	char id_str[10];

	cgiFormString("edit_id", id_str, sizeof(id_str));
	id_mod = atoi(id_str);

	cgiFormString("Sadd_name", s_name, sizeof(s_name));
	cgiFormString("Sadd_date", s_date, sizeof(s_date));
	cgiFormString("Sadd_height", s_height, sizeof(s_height));
	cgiFormString("Sadd_weight", s_weight, sizeof(s_weight));
	cgiFormString("gender_select", s_gender, sizeof(s_gender));
	cgiFormString("race_select", s_race, sizeof(s_race));
	cgiFormString("Sadd_area", s_area, sizeof(s_area));

	sprintf(sql_commond, "update M_studentlist set name='%s',genderID='%s',raceID='%s',height='%s',weight='%s',area='%s',date='%s' where id=%d", s_name, s_gender, s_race, s_height, s_weight, s_area, s_date, id_mod);

	ExecSelectQuery("manager", sql_commond, 0);
	cgiHeaderLocation("stu_mag.cgi?action_id=student_list");
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