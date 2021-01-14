using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using MySql.Data.MySqlClient;
using System.IO;

namespace Base
{
    internal class mysql
    {
        public static string database;
        public static string passwd;
        public static string serverAddr;
        public static string username;
        public static MySqlConnection iniHandle()
        {
            return new MySqlConnection(string.Format("Server={0};Database={1};Uid={2};Pwd={3};", new object[]
			{
				mysql.serverAddr,
				mysql.database,
				mysql.username,
				mysql.passwd
			}));
        }
        public static void iniLogin()
        {
            mysql.serverAddr = "localhost"; // don't change this
            mysql.database = "Base";//Don't change this ether the database name should be template
            mysql.username = "root";//Don't change this ether.    
            mysql.passwd = "";//Don't really need this.
        }

        public static bool open(MySqlConnection con)
        {
            try
            {
                con.Open();
                return true;
            }
            catch (MySqlException ex)
            {
                Base.write(string.Format("The server is restarting in two seconds due to {0}\n", ex.Message), " MYSQL ERR");
                Thread.Sleep(3000);
                Base.restart();
                return false;
            }
        }
    }
}
