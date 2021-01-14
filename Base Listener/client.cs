using System;
using System.Collections.Generic;
using System.Net;
using System.Globalization;
using System.IO;
using System.Linq;
using MySql.Data.MySqlClient;
using Security;


namespace Base
{
    public struct cData
    {
        public string cpukey;
        public string salt;
        public System.Net.EndPoint ep;
        public bool enabled;
        public byte[] kvdata;
        public string name;
        public string email;
        public DateTime time;
    }


    class client{
        public static string FirstCharToUpper(string value)
        {
            char[] array = value.ToCharArray();
            if (array.Length >= 1)
            {
                if (char.IsLower(array[0]))
                {
                    array[0] = char.ToUpper(array[0]);
                }
            }
            for (int i = 1; i < array.Length; i++)
            {
                if (array[i - 1] == ' ')
                {
                    if (char.IsLower(array[i]))
                    {
                        array[i] = char.ToUpper(array[i]);
                    }
                }
            }
            return new string(array);
        }

        public void cron_timeUpdate() {
            using (var con = mysql.iniHandle())
            using (var cmd = con.CreateCommand()) {
                if (!mysql.open(con)) return;
                cmd.CommandText = "UPDATE `consoles` SET `enabled`=false WHERE now() >= `time`";
                cmd.ExecuteNonQuery();
                con.Close();
            }
        }

        public void failConsole(string cpu, string ip, byte[] kv){
            using (var con = mysql.iniHandle())
            using (var cmd = con.CreateCommand()){
                if (!mysql.open(con)) return;
                cmd.CommandText = "insert into failed (cpukey, kvdata, num) values (@CpuKey, @kvdata, 0)";
                cmd.Parameters.AddWithValue("@CpuKey", cpu);
                cmd.Parameters.AddWithValue("@kvdata", kv);
                try
                {
                    cmd.ExecuteNonQuery();
                }
                catch (MySqlException ex)
                {
                    using (var newCmd = con.CreateCommand())
                    {
                        newCmd.CommandText = "UPDATE `failed` SET `kvdata` = @kvdata WHERE `cpukey` = @CpuKey";
                        newCmd.Parameters.AddWithValue("@CpuKey", cpu);
                        newCmd.Parameters.AddWithValue("@kvdata", kv);
                    }
                }
                con.Close();
            }
        }

        public bool getConsole(ref cData data, string cpukey){
            using (var con = mysql.iniHandle())
            using (var cmd = con.CreateCommand()){
                if (!mysql.open(con)) return false;
                cmd.CommandText = "select * from consoles where cpukey=@CpuKey";
                cmd.Parameters.AddWithValue("@CpuKey", cpukey);
                using(var rdr = cmd.ExecuteReader())
                if (rdr.Read()){
                    data.name = (string)rdr["name"];
                    data.email = (string)rdr["email"];
                    data.cpukey = (string)rdr["cpukey"];
                    data.enabled = (bool)rdr["enabled"];
                    data.time = (DateTime)rdr["time"];
                    data.salt = !rdr.IsDBNull(rdr.GetOrdinal("salt")) ? data.salt = (string)rdr["salt"] : data.salt = "";
                    data.kvdata = !rdr.IsDBNull(rdr.GetOrdinal("kvdata"))?(byte[])rdr["kvdata"]:data.kvdata = null;
                    con.Close();
                    return true;
                }
                con.Close();
            }
            return false;
        }

        public bool getConsoleBySession(ref cData data, string seshHash){
            using (var con = mysql.iniHandle())
            using (var cmd = con.CreateCommand()){
                if (!mysql.open(con)) return false;
                cmd.CommandText = "select * from consoles where salt=@Session";
                cmd.Parameters.AddWithValue("@Session", seshHash);
                using (var rdr = cmd.ExecuteReader())
                if (rdr.Read()){
                    data.name = (string)rdr["name"];
                    data.cpukey = (string)rdr["cpukey"];
                    data.enabled = (bool)rdr["enabled"];
                    data.time = (DateTime)rdr["time"];
                    data.salt = seshHash;
                    data.kvdata = !rdr.IsDBNull(rdr.GetOrdinal("kvdata"))?(byte[])rdr["kvdata"]:data.kvdata = null;
                    return true;
                }
            }
            return false;
        }

        public void SaveConsole(ref cData c)
        {
            using (var con = mysql.iniHandle())
            using (var cmd = con.CreateCommand()){
                if (!mysql.open(con)) return;
                cmd.CommandText = "update consoles set salt=@salt, enabled=@enabled, kvdata=@kvdata where cpukey=@cpukey;";
                cmd.Parameters.AddWithValue("@cpukey", c.cpukey);
                cmd.Parameters.AddWithValue("@salt", c.salt);
                int num = c.enabled ? 1 : 0;
                cmd.Parameters.AddWithValue("@enabled", num);
                if (c.kvdata != null) cmd.Parameters.AddWithValue("@kvdata", c.kvdata);
                else cmd.Parameters.AddWithValue("@kvdata", DBNull.Value);
                cmd.ExecuteNonQuery();
                con.Close();
            }
        }

    }
}