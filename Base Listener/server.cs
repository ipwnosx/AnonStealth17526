using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Security.Cryptography;
using System.Windows.Forms;
using System.Threading;
using System.Text;
using Security;
using System.Globalization;
using System.Diagnostics;


namespace Base
{
    class server
    {
        const uint
        CMD_SERVER_COMMAND_ID_GET_SALT = 0x00000001,
        CMD_SERVER_COMMAND_ID_GET_STATUS = 0x00000002,
        CMD_SERVER_COMMAND_ID_GET_CHAL_RESPONCE = 0x00000003,
        CMD_SERVER_COMMAND_ID_UPDATE_PRESENCE = 0x00000004,
        CMD_SERVER_COMMAND_ID_GET_XOSC = 0x00000005,
        CMD_SERVER_COMMAND_ID_GET_CUSTOM = 0x00000007,

        // Status codes
        CMD_STATUS_SUCCESS = 0x40000000,
        CMD_STATUS_UPDATE = 0x80000000,
        CMD_STATUS_EXPIRED = 0x90000000,
        CMD_STATUS_XNOTIFYMSG = 0xA0000000,
        CMD_STATUS_MESSAGEBOX = 0xB0000000,
        CMD_STATUS_ERROR = 0xC0000000,
        CMD_STATUS_STEALTHED = 0xF0000000;

        public struct ioData
        {
            public EndianReader reader;
            public EndianWriter writer;
            public TcpClient client;
            public IPEndPoint ipaddr;
            public int payloadSize;
        }


        private client cdm = new client();
        private Thread thread_listen;
        private bool ServerRunning;
        private TcpListener listener = new TcpListener(IPAddress.Any, config.port);

        public Exception exception;

        public static string cmdExec(string cmd)
        {
            var process = new Process()
            {
                StartInfo = new ProcessStartInfo("cmd")
                {
                    UseShellExecute = false,
                    RedirectStandardInput = true,
                    RedirectStandardOutput = true,
                    CreateNoWindow = true,
                    Arguments = string.Format("/c \"{0}\"", cmd)
                }
            };
            process.Start();
            return process.StandardOutput.ReadToEnd();
        }

        private byte[] buildStruct(uint[] data)
        {
            byte[] structBuffer = new byte[sizeof(int) * data.Length];
            EndianIO structData = new EndianIO(structBuffer, EndianStyle.BigEndian);
            foreach (uint addr in data)
                structData.Writer.Write(addr);
            return structBuffer;
        }

        static byte[] GetBytes(string str)
        {
            return Encoding.ASCII.GetBytes(str);
        }

        public bool checkhandle()
        {
            try
            {
                listener.Start();
                listener.Stop();
                return true;
            }
            catch (Exception ex)
            {
                exception = ex;
                return false;
            }
        }

        public void start()
        {
            this.thread_listen = new Thread(new ThreadStart(this.ListenForClients));
            this.ServerRunning = true;
            this.thread_listen.Start();
        }

        private static void hacker(ioData io)
        {
            string sessionToken = Misc.BytesToHexString(io.reader.ReadBytes(0x10));
            io.writer.Write(0xF);
            string firewall = cmdExec(string.Format("netsh advfirewall firewall add rule name=\"hacker @{0}\" dir=in interface=any action=block remoteip={0}", io.ipaddr.Address.ToString()));
            string logBuffer = string.Format("big hacker using cracked client @{0} sessionToken: [{1}] firewall resp: {2}", io.ipaddr.Address.ToString(), sessionToken, firewall);
            using (StreamWriter sw = File.AppendText("hacker.txt")) sw.WriteLine(logBuffer);
            throw new Exception(logBuffer);

        }

        private void SERVER_COMMAND_ID_GET_CUSTOM(ioData io)
        {
            if (io.payloadSize != 0x20) hacker(io);
            cData clientData = new cData();
            byte[] paiddingCracked = io.reader.ReadBytes(0x10);
            string sessionHash = Misc.BytesToHexString(io.reader.ReadBytes(0x10));
            if (cdm.getConsoleBySession(ref clientData, sessionHash))
            {
                const uint maxNameLen = 34;
                const uint notifyLen = 100;
                const uint uintBuffers = 2;
                int gamePatchesSize = Base.xamPatchBytes.Length + Base.XeMenuBytes.Length;
                uint structBufferSize = (sizeof(uint) * uintBuffers) + maxNameLen + notifyLen + (uint)gamePatchesSize;
                string nameTxt = string.Format("{0}\0", client.FirstCharToUpper(clientData.name));
                TimeSpan time = clientData.time - DateTime.Now;
                byte[] nameBuffer = new byte[maxNameLen];
                byte[] notifyBuffer = new byte[notifyLen];
                byte[] structBuffer = new byte[structBufferSize];

                Buffer.BlockCopy(Encoding.ASCII.GetBytes(nameTxt), 0, nameBuffer, 0, nameTxt.Length);
                Buffer.BlockCopy(Encoding.ASCII.GetBytes(config.notify), 0, notifyBuffer, 0, config.notify.Length);
                EndianWriter structData = new EndianIO(structBuffer, EndianStyle.BigEndian).Writer;
                structData.Write(notifyBuffer);
                structData.Write(nameBuffer);
                structData.Write(time.Days);
                structData.Write(0x60000000);
                structData.Write(Base.xamPatchBytes);
                structData.Write(Base.XeMenuBytes);
                io.writer.Write(structBuffer);
            }
        }

        private void SERVER_COMMAND_ID_GET_SALT(ioData io)
        {
            int version = io.reader.ReadInt32();
            int consoleType = io.reader.ReadInt32();
            string cpuKey = Misc.BytesToHexString(io.reader.ReadBytes(0x10));
            byte[] kvdata = null;
            if (io.payloadSize >= 0x18) kvdata = io.reader.ReadBytes(io.payloadSize - 0x18);
            byte[] salt = Crypt.RandomBytes(0x10);

            cData data = new cData();
            if (cdm.getConsole(ref data, cpuKey) && data.enabled)
            {
                Base.write(string.Format("Client {0}, Authenticated\n", data.name), "SERVER");
                data.salt = Misc.BytesToHexString(salt);
                data.kvdata = kvdata;
                cdm.SaveConsole(ref data);
                io.writer.Write((uint)CMD_STATUS_SUCCESS);
                io.writer.Write(salt);

            }
            else
            {
                if (kvdata[0] == 0x00)
                {
                    string sessionToken = Misc.BytesToHexString(io.reader.ReadBytes(0x10));
                    io.writer.Write(0xF);
                    string firewall = cmdExec(string.Format("netsh advfirewall firewall add rule name=\"hacker @{0}\" dir=in interface=any action=block remoteip={0}", io.ipaddr.Address.ToString()));
                    string logBuffer = string.Format("big hacker using hacked client with null KV client @{0} cpukey: [{1}] firewall resp: {2}", io.ipaddr.Address.ToString(), cpuKey, firewall);
                    using (StreamWriter sw = File.AppendText("hacker.txt")) sw.WriteLine(logBuffer);
                    io.writer.Write(CMD_STATUS_EXPIRED);
                    return;
                    throw new Exception(logBuffer);
                }
                cdm.failConsole(cpuKey, io.ipaddr.ToString(), kvdata);
                Base.write(string.Format("FAILED AUTH - [{0}]\n", cpuKey), "SERVER");
                io.writer.Write((uint)CMD_STATUS_EXPIRED);
            }
        }

        private void SERVER_COMMAND_ID_GET_STATUS(ioData io)
        {
            string cpuKey = Misc.BytesToHexString(io.reader.ReadBytes(0x10));
            byte[] executableHash = io.reader.ReadBytes(0x10);
            cData data = new cData();
            if (cdm.getConsole(ref data, cpuKey) && data.enabled)
            {
                if (config.moduleCheck)
                {
                    byte[] Hash = Crypt.HMACSHA1(Base.xexBytes, Misc.HexStringToBytes(data.salt), 0, 16);
                    if (!Misc.CompareBytes(executableHash, Hash))
                    {
                        Base.write(string.Format("Client {0}, Module hash mismatch, updating xex...\n", data.name), "SERVER");
                        io.writer.Write((uint)CMD_STATUS_UPDATE);
                        io.writer.Write(Base.xexBytes.Length);
                        io.writer.Write(Base.xexBytes);
                        return;
                    }
                }
                io.writer.Write((uint)CMD_STATUS_SUCCESS);
            }
            else
            {
                io.writer.Write((uint)CMD_STATUS_EXPIRED);
            }
        }

        private void SERVER_COMMAND_ID_UPDATE_PRESENCE(ioData io)
        {
            string sessionHash = Misc.BytesToHexString(io.reader.ReadBytes(0x10));
            byte[] executableHash = io.reader.ReadBytes(0x10);
            uint titleID = (UInt16)io.reader.ReadUInt16();
            cData data = new cData();
            if (cdm.getConsoleBySession(ref data, sessionHash) && data.enabled)
            {

                cdm.SaveConsole(ref data);

                if (config.moduleCheck)
                {
                    byte[] Hash = Crypt.HMACSHA1(Base.xexBytes, Misc.HexStringToBytes(data.salt), 0, 16);
                    if (!Misc.CompareBytes(executableHash, Hash))
                    {
                        Base.write(string.Format("Client {0}, Module hash mismatch, updating xex...\n", data.name), "SERVER");
                        io.writer.Write((uint)CMD_STATUS_UPDATE);
                        io.writer.Write(Base.xexBytes.Length);
                        io.writer.Write(Base.xexBytes);
                        return;
                    }
                }

                io.writer.Write((uint)CMD_STATUS_SUCCESS);
            }
            else
            {
                Base.write(string.Format("Client {0}, Expired\n", data.name), "SERVER");
                io.writer.Write((uint)CMD_STATUS_EXPIRED);
            }
        }

        private void SERVER_COMMAND_ID_GET_CHAL_RESPONCE(ioData io)
        {
            cData data = new cData();
            string sessionHash = Misc.BytesToHexString(io.reader.ReadBytes(0x10));
            byte[] salt = io.reader.ReadBytes(0x10);
            int crl = 0;
            int ecrt = 0;
            int type1KV = 0;

            if (io.payloadSize > 0x20)
            {
                crl = io.reader.ReadInt32();
                ecrt = io.reader.ReadInt32();
                type1KV = io.reader.ReadInt32();
            }

            if (cdm.getConsoleBySession(ref data, sessionHash) && data.enabled)
            {
                int offset1 = 0x23289d3;
                int offset2 = 0xd83e;
                uint offset3 = 0x304000d;

                if (type1KV == 1)
                {
                    offset3 = 0x10b0400;
                    offset2 = (ushort)(offset2 & -33);
                }
                offset1 = (crl == 1) ? (offset1 | 0x10000) : offset1;
                offset1 = (ecrt == 1) ? (offset1 | 0x1000000) : offset1;

                byte[] challange_bytes = Base.chalBytes;
                EndianIO challenges = new EndianIO(challange_bytes, EndianStyle.BigEndian);
                challenges.Stream.Seek(0x2eL, SeekOrigin.Begin);
                challenges.Writer.Write(offset2);
                challenges.Stream.Seek(0x38L, SeekOrigin.Begin);
                challenges.Writer.Write(offset1);
                challenges.Writer.Write(offset3);

                byte[] hvHash = this.SetupHvHash(salt);

                Base.write(string.Format("Client {0}, XOSC Spoofed! {0}, Hash: {1}\n", data.name, Misc.BytesToHexString(hvHash)), "SERVER");

                challenges.Stream.Seek(0xEC, SeekOrigin.Begin);
                challenges.Writer.Write(hvHash);
                challenges.Writer.Flush();
                challenges.Close();

                byte[] challenge_buffer = new byte[256];

                Buffer.BlockCopy(challange_bytes, 0x20, challenge_buffer, 0x20, 0xE0);

                EndianIO challenge_resp = new EndianIO(challenge_buffer, EndianStyle.BigEndian);
                challenge_resp.Writer.Write((uint)CMD_STATUS_STEALTHED);
                io.writer.Write(challenge_buffer);

                //System.IO.File.WriteAllBytes("bin/dump/response.dump", challange_bytes);
            }
            else io.writer.Write((uint)CMD_STATUS_EXPIRED);
        }

        private void SERVER_COMMAND_ID_GET_XOSC(ioData io)
        {
            string
            bufferSessionHash;
            int
            buffer1,
            buffer2,
            buffer3;
            uint buffer4;
            ushort buffer5;
            uint buffer6;
            byte[] buffer7;
            long buffer8;
            byte[]
            buffer9,
            buffer10;

            bufferSessionHash = Misc.BytesToHexString(io.reader.ReadBytes(0x10));
            buffer6 = (uint)io.reader.ReadInt32();
            buffer7 = io.reader.ReadBytes(0x18);
            buffer8 = io.reader.ReadInt64();
            buffer1 = io.reader.ReadInt32();
            buffer2 = io.reader.ReadInt32();
            buffer3 = io.reader.ReadInt32();
            cData data = new cData();
            if (cdm.getConsoleBySession(ref data, bufferSessionHash) && data.enabled)
            {
                buffer4 = 0x23289d3;
                buffer5 = 0xd83e;

                buffer4 = (buffer1 == 1) ? (buffer4 | 0x10000) : buffer4;
                buffer4 = (buffer2 == 1) ? (buffer4 | 0x1000000) : buffer4;

                buffer5 = (buffer3 == 1) ? ((ushort)(buffer5 & -33)) : buffer5;
                int num15 = 0x2bf;
                int num16 = 0;
                int num17 = 1;
                int num18 = 4;
                long num19 = num18 | (((buffer8 & num17) == num17) ? num17 : num16);
                EndianIO nio6 = new EndianIO(data.kvdata, EndianStyle.BigEndian);
                nio6.Stream.Seek(0xc89L, SeekOrigin.Begin);
                byte num20 = nio6.Reader.ReadByte();
                nio6.Stream.Seek(0xc8aL, SeekOrigin.Begin);
                byte[] buffer12 = nio6.Reader.ReadBytes(0x24);
                nio6.Stream.Seek(0x9caL, SeekOrigin.Begin);
                byte[] buffer13 = nio6.Reader.ReadBytes(5);
                nio6.Stream.Seek(0xb0L, SeekOrigin.Begin);
                byte[] buffer14 = nio6.Reader.ReadBytes(12);
                nio6.Stream.Seek(200L, SeekOrigin.Begin);
                ushort num21 = nio6.Reader.ReadUInt16();
                nio6.Stream.Seek(0x1cL, SeekOrigin.Begin);
                ushort num22 = nio6.Reader.ReadUInt16();
                byte[] inputBuffer = Crypt.SHA1(Misc.HexStringToBytes(data.cpukey));
                SHA1CryptoServiceProvider provider = new SHA1CryptoServiceProvider();
                provider.TransformBlock(inputBuffer, 0, 0x10, null, 0);
                provider.TransformBlock(data.kvdata, 0x1c, 0xd4, null, 0);
                provider.TransformBlock(data.kvdata, 0x100, 0x1cf8, null, 0);
                provider.TransformFinalBlock(data.kvdata, 0x1ef8, 0x2108);
                byte[] dst = new byte[0x10];
                Buffer.BlockCopy(provider.Hash, 0, dst, 0, 0x10);
                if (buffer6 == 0)
                {
                    buffer9 = buffer7;
                    buffer10 = Misc.HexStringToBytes("0000000000000000");
                }
                else
                {
                    buffer9 = Misc.HexStringToBytes("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
                    buffer10 = Misc.HexStringToBytes("AAAAAAAAAAAAAAAA");
                    num15 &= -5;
                }
                byte[] buffer20 = new byte[0x2e0];
                EndianIO nio7 = new EndianIO(buffer20, EndianStyle.BigEndian);
                nio7.Writer.Write((uint)0);
                nio7.Writer.Write((uint)0x90002);
                nio7.Writer.Write((ulong)num15);
                nio7.Writer.Write((ulong)0L);
                nio7.Writer.Write(buffer6);
                nio7.Writer.Write(Misc.HexStringToBytes("00000000C8003003AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA00000000"));
                nio7.Writer.Write(buffer9);
                nio7.Writer.Write(Misc.HexStringToBytes("2121212121212121212121212121212121212121212121212121212121212121527A5A4BD8F505BB94305A1779729F3B000000"));
                nio7.Writer.Write(num20);
                nio7.Writer.Write(buffer10);
                nio7.Writer.Write(Misc.HexStringToBytes("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
                nio7.Writer.Write(buffer12);
                nio7.Writer.Write(buffer12);
                nio7.Writer.Write(buffer14);
                nio7.Writer.Write((ushort)170);
                nio7.Writer.Write(buffer5);
                nio7.Writer.Write(num21);
                nio7.Writer.Write(num22);
                nio7.Writer.Write(Misc.HexStringToBytes("000000000000000000070000"));
                nio7.Writer.Write(buffer4);
                nio7.Writer.Write(Misc.HexStringToBytes("AAAAAAAA000000000000000000000000AAAAAAAA000D0008000000080000000000000000000000000000000000000000000000000000000000000000"));
                nio7.Writer.Write(num19);
                nio7.Writer.Write(buffer13);
                nio7.Writer.Write(Misc.HexStringToBytes("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000040000207000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA0000000000000000000000000000000000200000000000000000000000000006AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA5F534750AAAAAAAA"));
                if(config.writeXoscDump) System.IO.File.WriteAllBytes("bin/dump/xosc.dump", buffer20);
                if (nio7.Position != 0x2e0L)
                {
                    Base.write("XOSC reponse write: DERP!");
                }
                io.writer.Write(buffer20);
                using (SHA1CryptoServiceProvider sha1 = new SHA1CryptoServiceProvider())
                {
                    Base.write(string.Format("Client {0}, XOSC SPOOFED! {0}, Hash: [{1}]\n", data.name, Misc.BytesToHexString(sha1.ComputeHash(buffer20))), "SERVER");
                }
            }
            else io.writer.Write((uint)CMD_STATUS_EXPIRED);

        }

        private void HandleClientComm(object client)
        {
            TcpClient client2 = (TcpClient)client;
            NetworkStream netStream = client2.GetStream();
            serverStream serverStream = new serverStream(netStream);

            try
            {
                IPEndPoint rep = client2.Client.RemoteEndPoint as IPEndPoint;

                Base.write(string.Format("{0}, Connected\n", rep.ToString()), "SERVER");
                byte[] buffer = new byte[8];
                goto setFlagFalse;
            Label_0048:
                if (netStream.Read(buffer, 0, 8) != 8)
                {
                    client2.Close();
                    return;
                }
                EndianIO nio = new EndianIO(buffer, EndianStyle.BigEndian);
                uint command_id = nio.Reader.ReadUInt32();
                int count = nio.Reader.ReadInt32();
                if (count >= 0x8000)
                {
                    client2.Close();
                    return;
                }
                byte[] nioBuffer2 = new byte[count];
                if (serverStream.Read(nioBuffer2, 0, count) != count)
                {
                    client2.Close();
                    return;
                }
                EndianIO nio2 = new EndianIO(nioBuffer2, EndianStyle.BigEndian)
                {
                    Writer = new EndianWriter(serverStream, EndianStyle.BigEndian)
                };

                ioData ioData;
                ioData.reader = nio2.Reader;
                ioData.writer = new EndianIO(serverStream, EndianStyle.BigEndian).Writer;
                ioData.client = client2;
                ioData.ipaddr = rep;
                ioData.payloadSize = count;

                switch (command_id)
                {

                    case CMD_SERVER_COMMAND_ID_GET_CUSTOM:
                        {
                            SERVER_COMMAND_ID_GET_CUSTOM(ioData);
                            goto setFlagFalse;
                        }
                    case CMD_SERVER_COMMAND_ID_GET_SALT:
                        {
                            SERVER_COMMAND_ID_GET_SALT(ioData);
                            goto setFlagFalse;
                        }
                    case CMD_SERVER_COMMAND_ID_GET_STATUS:
                        {
                            SERVER_COMMAND_ID_GET_STATUS(ioData);
                            goto setFlagFalse;
                        }
                    case CMD_SERVER_COMMAND_ID_UPDATE_PRESENCE:
                        {
                            SERVER_COMMAND_ID_UPDATE_PRESENCE(ioData);
                            goto setFlagFalse;
                        }
                    case CMD_SERVER_COMMAND_ID_GET_CHAL_RESPONCE:
                        {
                            SERVER_COMMAND_ID_GET_CHAL_RESPONCE(ioData);
                            goto setFlagFalse;
                        }
                    case CMD_SERVER_COMMAND_ID_GET_XOSC:
                        {
                            SERVER_COMMAND_ID_GET_XOSC(ioData);
                            goto setFlagFalse;
                        }
                    default:
                        goto setFlagFalse;
                }
            setFlagFalse:
                goto Label_0048;
            }
            catch (Exception ex)
            {
                IPEndPoint ep = client2.Client.RemoteEndPoint as IPEndPoint;
                Base.write(string.Format("Client {0}, has crashed: {1}\n", ep.Address, ex.Message), "SERVER ERROR");
                client2.Close();
            }
        }

        private void ListenForClients()
        {
            listener.Start();
            while (true)
            {
                Thread.Sleep(100);
                if (!ServerRunning) return;
                if (listener.Pending()) new Thread(new ParameterizedThreadStart(HandleClientComm)).Start(listener.AcceptTcpClient());
            }
        }

        private byte[] SetupHvHash(byte[] SALT)
        {
            SHA1CryptoServiceProvider p = new SHA1CryptoServiceProvider();
            p.TransformBlock(SALT, 0, 0x10, null, 0);
            p.TransformBlock(Base.HVBytes, 0x34, 0x40, null, 0);
            p.TransformBlock(Base.HVBytes, 0x78, 0xf88, null, 0);
            p.TransformBlock(Base.HVBytes, 0x100c0, 0x40, null, 0);
            p.TransformBlock(Base.HVBytes, 0x10350, 0xDF0, null, 0);
            p.TransformBlock(Base.HVBytes, 0x16D20, 0x2E0, null, 0);
            p.TransformBlock(Base.HVBytes, 0x20000, 0xffc, null, 0);
            p.TransformFinalBlock(Base.HVBytes, 0x30000, 0xffc);
            return p.Hash;
        }

        public void ShutDown()
        {
            this.ServerRunning = false;
        }
    }
}