using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Threading;
using MySql.Data.MySqlClient;
using System.IO;
using System.Runtime.InteropServices;

namespace Base {

    class Base{
        const Int32 SW_MINIMIZE = 6;

        [DllImport("Kernel32.dll", CallingConvention = CallingConvention.StdCall, SetLastError = true)]
        private static extern IntPtr GetConsoleWindow();

        [DllImport("User32.dll", CallingConvention = CallingConvention.StdCall, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool ShowWindow([In] IntPtr hWnd, [In] Int32 nCmdShow);


        public static byte[] HVBytes = null;
        public static byte[] chalBytes = null;
        public static byte[] xexBytes = null;
        public static byte[] xamPatchBytes = null;
        public static byte[] XeMenuBytes = null;
        public static string[] conf;
        public static string XeXName = "bin/Base.xex";
        public static string hypervisor = "bin/HV.bin";
        public static string chall_resp = "bin/chall_resp.bin";
        public static string xamPatchData = "bin/xamPatchFile.bin";
        public static string XeMenuName = "bin/Notify.xex";
        public static string notify = "notify.txt";

        private static bool enable_cronRestart = false;

        private static void MinimizeConsoleWindow()
        {
            IntPtr hWndConsole = GetConsoleWindow();
            ShowWindow(hWndConsole, SW_MINIMIZE);
        }

        static void Main(string[] args){
            if (config.crashRecovery)
            {
                Application.ThreadException += new ThreadExceptionEventHandler(exception_appThread);
                AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(exception_curdomainUnhandled);
            }

            config.writeDelogLog = File.Exists(config.writeDelogLogKey);

            
            Console.Title = "Base: waiting..";
            Console.BackgroundColor = ConsoleColor.Black;
            Console.ForegroundColor = ConsoleColor.Green;
            Console.BufferWidth = 120;
            Console.Clear();
            if (Initialize()) write("-------------------------------------------\n");
            else{
                write("Service Haulted! Initialize Failure\n", "SYS");
                write("Restarting in 5 seconds...\n", "SYS");
                Thread.Sleep(5000);
                restart();
            }
            MinimizeConsoleWindow();
        }


        private static bool Initialize(){
            write(string.Format("Reading conf.ini "), "SERVER");
            try { 
                conf = File.ReadAllLines("conf.ini");
                int portBase = int.Parse(conf[1]);
                config.port = 0x7D0 + portBase;
                config.cmdPort = 0x7D0 + (int.Parse(conf[1]) + 500);
                config.SERVERNAME = conf[0];
                config.privateServer = true;
                write("SUCCESS!\n");
            }
            catch
            {
                write("SKIPPED\n");
            }

            mysql.iniLogin();
            write(string.Format("Authenticating to MySQL server [{0}]... ", mysql.serverAddr.Substring(0, 3)), "SERVER");
            using (var con = mysql.iniHandle())
            try{
                con.Open();
            }catch (MySqlException ex){
                write(string.Format("FAILED! #{0}\n", ex.Number));
                write(string.Format("{0}\n", ex.Message), "MYSQL");
                return false;
            }
            write("SUCCESS!\n");

            server svr = new server();
            write(string.Format("Server binding to port {0}... ", config.port), "SERVER");
            if (!svr.checkhandle()){
                write(string.Format("FAILED - PORT IN USE {0}\n", svr.exception.Message));
                return false;
            }else{
                write(string.Format("SUCCESS!\n", config.port));
                svr.start();
            }

            cmdServer cmdSvr = new cmdServer();
            write(string.Format("Command server binding to port {0}... ", config.cmdPort), "SERVER"); ;
            if (!cmdSvr.checkhandle()){
                write("FAILED - PORT IN USE\n");
                return false;
            }else{
                write("SUCCESS!\n");
                cmdSvr.start();
            }

            write(string.Format("Reading bytes from ({0})... ", hypervisor), "SERVER");
            try{HVBytes = System.IO.File.ReadAllBytes(hypervisor);}catch{
                write("FAILED - FILE NOT FOUND\n");
                return false;
            }write("SUCCESS!\n");

            write(string.Format("Reading bytes from ({0})... ", chall_resp), "SERVER");
            try{chalBytes = System.IO.File.ReadAllBytes(chall_resp);}catch{
                write("FAILED - FILE NOT FOUND\n");
                return false;
            }write("SUCCESS!\n");

            write(string.Format("Reading bytes from ({0})... ", XeXName), "SERVER");
            try { xexBytes = System.IO.File.ReadAllBytes(XeXName); }
            catch{
                write("FAILED - FILE NOT FOUND\n");
                return false;
            } write("SUCCESS!\n");

            write(string.Format("Reading bytes from ({0})... ", xamPatchData), "SERVER");
            try { xamPatchBytes = System.IO.File.ReadAllBytes(xamPatchData); }
            catch
            {
                write("FAILED - FILE NOT FOUND\n");
                return false;
            } write(string.Format("0x{0} SUCCESS!\n", xamPatchBytes.Length.ToString("X")));

            Base.write(string.Format("Reading bytes from ({0})... ", XeMenuName), "SERVER");
            try { XeMenuBytes = File.ReadAllBytes(XeMenuName); }
            catch
            {
                write("FAILED - FILE NOT FOUND\n", null);
                return false;
            } Base.write(string.Format("0x{0} SUCCESS!\n", Base.XeMenuBytes.Length.ToString("X")), null);

            write(string.Format("Reading string from ({0})... ", notify), "SERVER");
            try
            {
                string notifyBuffer = File.ReadAllLines(notify)[0];
                if(!string.IsNullOrEmpty(notifyBuffer) && notifyBuffer.Length<99) config.notify = notifyBuffer + "\0";
                else throw new Exception();
                write("SUCCESS!\n");
            }
            catch
            {
                write("SKIPPED\n");
            }

            Console.Title = string.Format("Base: listening on port {0} - {1}", config.port, config.privateServer?config.SERVERNAME:"StealthProject");
            Console.SetWindowSize(120, 40);
            new Thread(new ThreadStart(event_cronTimeUpdate)).Start();
            if (enable_cronRestart) new Thread(new ThreadStart(event_cronRestart)).Start();
            return true;
        }

        static void exception_appThread(object snd, ThreadExceptionEventArgs ex){
            write(string.Format("Server restarting due to unhandled exception thrown: {0}\n", ex.Exception.Message), "UNHANDLED EXCEPTION");
            Thread.Sleep(3000);
            restart();
        }

        static void exception_curdomainUnhandled(object snd, UnhandledExceptionEventArgs e){
            write("Server restarting due to a current domain unhandled exception thrown", "UNHANDLED EXCEPTION");
            Thread.Sleep(3000);
           restart();
        }

        public static void writelog(string txt){
            if (!File.Exists(config.logdir)) using (StreamWriter sw = File.CreateText(config.logdir)) sw.WriteLine("File created");
            using (StreamWriter sw = File.AppendText(config.logdir)) sw.WriteLine(txt);
        }

        public static void write(string txt, string tag = null){
            if (!config.output) return;
            string time = DateTime.Now.ToString("h:mm:sstt");
            string outp = tag==null?txt:string.Format("[{0} {1}] {2}", time, tag, txt);
            Console.Write(outp);
            if (config.writeDelogLog) writelog(outp);
        }

        public static void write2(string format, params object[] args){
            Console.Write(format, args);
        }

        public static void event_cronTimeUpdate() {
            for (; ; ){
                client cm = new client();
                cm.cron_timeUpdate();
                Thread.Sleep(7 * 1000);
            }
        }

        public static void event_cronRestart(){
            Thread.Sleep(700000);
            write("Server restarting it self...\n", "SERVER");
            restart();
        }

        public static void restart(){
            write("Server restarting\n", "SVR");
            System.Diagnostics.Process.Start(Application.ExecutablePath);
            Environment.Exit(0);
        }
    }
}
