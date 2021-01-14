using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Base
{
    class config
    {
        public static bool moduleCheck = true;
        public static bool crashRecovery = true;
        public static string SERVERNAME = "Base";
        public static bool privateServer = false;
        public static int port = 9000;
        public static int cmdPort = port + 1;
        public static string notify = "Base - Success" + "\0";
        public static string logdir = "DebugOutput.log";
        public static string writeDelogLogKey = "ENABLE_DEBUG";
        public static bool output = true;
        public static bool writeXoscDump = false;
        public static bool writeDelogLog;
    }
}
