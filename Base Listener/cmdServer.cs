using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Net.Sockets;
using System.Net;

namespace Base
{
    class cmdServer
    {
        private string auth_token = "PUT YOUR OWN HASH HERE";
        public Exception exception;
        private TcpListener server = new TcpListener(IPAddress.Any, config.cmdPort);

        public void start(){
            new Thread(new ThreadStart(cmdServerThread)).Start();
        }

        public bool checkhandle(){
            try{
                server.Start();
                server.Stop();
                return true;
            }catch (Exception ex){
                exception = ex;
                return false;
            }
        }

        public bool execCmd(string cmd, string ip){
            switch (cmd){
                case "clearc":
                    Console.Clear();
                    Base.write(string.Format("Admin [{0}] cleared console...\n", ip), "CMDSERVER");
                    break;
                case "output":
                    config.output = !config.output;
                    Base.write(string.Format("Output set: {0}\n", config.output ? "TRUE" : "FALSE"), "CMDSERVER");
                    Base.write(string.Format("Admin [{0}] toggled output {0}...\n", ip, config.output ? "TRUE" : "FALSE"), "CMDSERVER");
                    break;
                case "restart":
                    Base.write(string.Format("Admin [{0}] restarted server...\n", ip), "CMDSERVER");
                    Base.restart();
                    break;
                case "kill":
                    Base.write(string.Format("SERVER KILLED\n", ip), "CMDSERVER");
                    Environment.Exit(0);
                    break;
                default: return false;
            }
            return true;
        }

        public void cmdServerThread(){
            try
            {
                server.Start();
                const int token = 0, cmd = 1, ip = 2;
                Byte[] bytes = new Byte[256];
                String data = null;
                while (true)
                {
                    if (server.Pending())
                    {
                        int i;
                        TcpClient client = server.AcceptTcpClient();
                        data = null;
                        NetworkStream stream = client.GetStream();
                        while ((i = stream.Read(bytes, 0, bytes.Length)) != 0)
                        {
                            data = System.Text.Encoding.ASCII.GetString(bytes, 0, i);
                            string[] args = data.Split(';');
                            string ipaddr = client.Client.RemoteEndPoint.ToString();
                            if (args.Count() == 3)
                            {
                                if (args[token].Equals(auth_token))
                                {
                                    if (args[cmd].Equals("ping")) stream.Write(Encoding.ASCII.GetBytes("pong"), 0, 0x4);
                                    else if (!execCmd(args[cmd], args[ip])) stream.Write(Encoding.ASCII.GetBytes("Invalid Command"), 0, 15);
                                    Base.write(string.Format("{0} - [{1}]\n", args[ip], args[cmd]), "CMDSERVER");
                                }
                                else Base.write(string.Format("{0} invalid token\n", ipaddr), "CMDSERVER");
                            }
                            else Base.write(string.Format("{0} invalid buffer\n", ipaddr), "CMDSERVER");
                        } client.Close();
                    } Thread.Sleep(500);
                }
            }
            catch { }
        }
    }

}
