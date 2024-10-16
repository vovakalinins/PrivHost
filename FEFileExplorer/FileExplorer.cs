using System;
using System.Diagnostics;
using System.Windows.Forms;
using Microsoft.Toolkit.Uwp.Notifications;


namespace FEFileExplorer
{
    public partial class FileExplorer : Form
    {
        static ServerCommunicator communicator = new ServerCommunicator();
        public FileExplorer()
        {
            InitializeComponent();
            _ = communicator.ListFiles(fileListView);
        }

        private async void fileListView_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            ListViewHitTestInfo hit = fileListView.HitTest(e.Location);
            if (hit.Item != null)
            {
                string filename = hit.Item.Text;

                await communicator.DownloadFileWithProgress(filename, downloadProgress);

                new ToastContentBuilder()
                    .AddText("Download completed!")
                    .AddText($"The file {filename} has been downloaded successfully.")
                    .Show();
            }
        }

        private void uploadToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void openDownloadFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Process.Start("downloads");
        }
        private void reloadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            _ = communicator.ListFiles(fileListView);
        }
        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Environment.Exit(0);
        }
    }
}
