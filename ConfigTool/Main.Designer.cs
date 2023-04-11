namespace DLSSTweaks.ConfigTool
{
    partial class Main
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            loadToolStripMenuItem = new ToolStripMenuItem();
            saveToolStripMenuItem = new ToolStripMenuItem();
            menuStrip1 = new MenuStrip();
            addDLLOverrideToolStripMenuItem = new ToolStripMenuItem();
            lvSettings = new ListViewEx();
            columnHeader1 = new ColumnHeader();
            columnHeader2 = new ColumnHeader();
            splitContainer1 = new SplitContainer();
            txtDesc = new TextBox();
            menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
            splitContainer1.Panel1.SuspendLayout();
            splitContainer1.Panel2.SuspendLayout();
            splitContainer1.SuspendLayout();
            SuspendLayout();
            // 
            // loadToolStripMenuItem
            // 
            loadToolStripMenuItem.Name = "loadToolStripMenuItem";
            loadToolStripMenuItem.Size = new Size(76, 34);
            loadToolStripMenuItem.Text = "Load";
            loadToolStripMenuItem.MouseHover += loadToolStripMenuItem_MouseHover;
            // 
            // saveToolStripMenuItem
            // 
            saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            saveToolStripMenuItem.Size = new Size(74, 34);
            saveToolStripMenuItem.Text = "Save";
            saveToolStripMenuItem.Click += saveToolStripMenuItem_Click;
            saveToolStripMenuItem.MouseHover += saveToolStripMenuItem_MouseHover;
            // 
            // menuStrip1
            // 
            menuStrip1.ImageScalingSize = new Size(20, 20);
            menuStrip1.Items.AddRange(new ToolStripItem[] { loadToolStripMenuItem, saveToolStripMenuItem, addDLLOverrideToolStripMenuItem });
            menuStrip1.Location = new Point(0, 0);
            menuStrip1.Name = "menuStrip1";
            menuStrip1.Padding = new Padding(9, 3, 0, 3);
            menuStrip1.Size = new Size(975, 40);
            menuStrip1.TabIndex = 0;
            menuStrip1.Text = "menuStrip1";
            // 
            // addDLLOverrideToolStripMenuItem
            // 
            addDLLOverrideToolStripMenuItem.Name = "addDLLOverrideToolStripMenuItem";
            addDLLOverrideToolStripMenuItem.Size = new Size(195, 34);
            addDLLOverrideToolStripMenuItem.Text = "Add DLL Override";
            addDLLOverrideToolStripMenuItem.Click += addDLLOverrideToolStripMenuItem_Click;
            addDLLOverrideToolStripMenuItem.MouseHover += addDLLOverrideToolStripMenuItem_MouseHover;
            // 
            // lvSettings
            // 
            lvSettings.Columns.AddRange(new ColumnHeader[] { columnHeader1, columnHeader2 });
            lvSettings.Dock = DockStyle.Fill;
            lvSettings.FullRowSelect = true;
            lvSettings.Location = new Point(0, 0);
            lvSettings.Margin = new Padding(4);
            lvSettings.Name = "lvSettings";
            lvSettings.Size = new Size(975, 526);
            lvSettings.TabIndex = 1;
            lvSettings.UseCompatibleStateImageBehavior = false;
            lvSettings.View = View.Details;
            lvSettings.ItemMouseHover += lvSettings_ItemMouseHover;
            lvSettings.SelectedIndexChanged += lvSettings_SelectedIndexChanged;
            // 
            // columnHeader1
            // 
            columnHeader1.Text = "Setting";
            columnHeader1.Width = 250;
            // 
            // columnHeader2
            // 
            columnHeader2.Text = "Value";
            columnHeader2.Width = 250;
            // 
            // splitContainer1
            // 
            splitContainer1.Dock = DockStyle.Fill;
            splitContainer1.Location = new Point(0, 40);
            splitContainer1.Margin = new Padding(4);
            splitContainer1.Name = "splitContainer1";
            splitContainer1.Orientation = Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            splitContainer1.Panel1.Controls.Add(lvSettings);
            // 
            // splitContainer1.Panel2
            // 
            splitContainer1.Panel2.Controls.Add(txtDesc);
            splitContainer1.Size = new Size(975, 869);
            splitContainer1.SplitterDistance = 526;
            splitContainer1.SplitterWidth = 6;
            splitContainer1.TabIndex = 3;
            // 
            // txtDesc
            // 
            txtDesc.Dock = DockStyle.Fill;
            txtDesc.Location = new Point(0, 0);
            txtDesc.Margin = new Padding(4);
            txtDesc.Multiline = true;
            txtDesc.Name = "txtDesc";
            txtDesc.ReadOnly = true;
            txtDesc.ScrollBars = ScrollBars.Vertical;
            txtDesc.Size = new Size(975, 337);
            txtDesc.TabIndex = 5;
            // 
            // Main
            // 
            AutoScaleDimensions = new SizeF(12F, 30F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(975, 909);
            Controls.Add(splitContainer1);
            Controls.Add(menuStrip1);
            MainMenuStrip = menuStrip1;
            Margin = new Padding(4);
            Name = "Main";
            Text = "DLSSTweaks ConfigTool";
            menuStrip1.ResumeLayout(false);
            menuStrip1.PerformLayout();
            splitContainer1.Panel1.ResumeLayout(false);
            splitContainer1.Panel2.ResumeLayout(false);
            splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
            splitContainer1.ResumeLayout(false);
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private ToolStripMenuItem loadToolStripMenuItem;
        private ToolStripMenuItem saveToolStripMenuItem;
        private MenuStrip menuStrip1;
        private ListViewEx lvSettings;
        private ColumnHeader columnHeader1;
        private ColumnHeader columnHeader2;
        private SplitContainer splitContainer1;
        private TextBox txtDesc;
        private ToolStripMenuItem addDLLOverrideToolStripMenuItem;
    }
}