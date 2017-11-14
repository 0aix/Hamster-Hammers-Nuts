using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MeshTool
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(Application.ExecutablePath).TrimEnd('\\') + "\\blends";
        }

        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.FileName = "blender.exe";
            startInfo.Arguments = "--background --python export-mesh.py --";
            startInfo.UseShellExecute = false;
            foreach (string path in openFileDialog1.FileNames)
            {
                if (path.EndsWith(".blend"))
                {
                    startInfo.Arguments += " " + path;
                }
            }
            Process.Start(startInfo);
            Application.Exit();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            openFileDialog1.ShowDialog();
        }
    }
}
