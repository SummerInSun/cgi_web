1. File description
student:
	student_addnew.html		//As it says: add the students or edit the students
	student_list.html		//The students list from mysql
	stu_mag.cgi				//Manage the students, CGI execution
teacher:
	teacher_addnew.html		//As it says: add the teachers or edit the teachers
	teacher_list.html		//The teachers list from mysql
	tech_mag.cgi			//Manage the teachers, CGI execution
netdisk:
	netdisk_list.html		//List the upload files, include delete and flush the files
	netdisk.cgi				//Manage the netdisk, CGI execution
clear.bat:
	//open it with notepad
	
CSS(dir):
	table.css				//CSS files
netdisk(dir):
	...
	...						//Upload files
stu_mag(dir):
	...
	...						//VS2012 projects, you should change the path of output
teac_mag(dir):
	...
	...						//VS2012 projects, you should change the path of output
netdiskpro(dir):
	...
	...						//VS2012 projects, you should change the path of output
	
2. How to use it
mysql: 	name:root, password:root, 
		sqlname:manager
			table:
				ms_races:id races
				ms_genders:id gender
				m_teacherlist:id name gender phone
				m_studentlist:id name genderID raceID height weight area date
				m_netdisklist:id f_name f_size f_time f_owner
			
3. bugs

#There have no input control for every input forms
#If upload one file much than one time, the list html will have two records, but in the netdis(dir), there is one file
#... more          