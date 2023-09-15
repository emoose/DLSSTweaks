using System.Drawing;
using System.Windows.Forms;

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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Main));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addDLLOverrideToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.installToGameFolderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.lvSettings = new DLSSTweaks.ConfigTool.ListViewEx();
            this.chSetting = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.chValue = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.menuSettings = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.ctxRename = new System.Windows.Forms.ToolStripMenuItem();
            this.ctxDelete = new System.Windows.Forms.ToolStripMenuItem();
            this.txtDesc = new System.Windows.Forms.TextBox();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.lblIniPath = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.menuSettings.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.GripMargin = new System.Windows.Forms.Padding(2, 2, 0, 2);
            this.menuStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.saveToolStripMenuItem,
            this.loadToolStripMenuItem,
            this.addDLLOverrideToolStripMenuItem,
            this.installToGameFolderToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Padding = new System.Windows.Forms.Padding(7, 4, 0, 4);
            this.menuStrip1.ShowItemToolTips = true;
            this.menuStrip1.Size = new System.Drawing.Size(856, 42);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Image = global::DLSSTweaks.ConfigTool.Properties.Resources.save;
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(94, 34);
            this.saveToolStripMenuItem.Text = "Save";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
            this.saveToolStripMenuItem.MouseHover += new System.EventHandler(this.saveToolStripMenuItem_MouseHover);
            // 
            // loadToolStripMenuItem
            // 
            this.loadToolStripMenuItem.Image = global::DLSSTweaks.ConfigTool.Properties.Resources.refresh;
            this.loadToolStripMenuItem.Name = "loadToolStripMenuItem";
            this.loadToolStripMenuItem.Size = new System.Drawing.Size(114, 34);
            this.loadToolStripMenuItem.Text = "Reload";
            this.loadToolStripMenuItem.Click += new System.EventHandler(this.loadToolStripMenuItem_Click);
            this.loadToolStripMenuItem.MouseHover += new System.EventHandler(this.loadToolStripMenuItem_MouseHover);
            // 
            // addDLLOverrideToolStripMenuItem
            // 
            this.addDLLOverrideToolStripMenuItem.Image = global::DLSSTweaks.ConfigTool.Properties.Resources.dlloverride;
            this.addDLLOverrideToolStripMenuItem.Name = "addDLLOverrideToolStripMenuItem";
            this.addDLLOverrideToolStripMenuItem.Size = new System.Drawing.Size(215, 34);
            this.addDLLOverrideToolStripMenuItem.Text = "Add DLL Override";
            this.addDLLOverrideToolStripMenuItem.Click += new System.EventHandler(this.addDLLOverrideToolStripMenuItem_Click);
            this.addDLLOverrideToolStripMenuItem.MouseHover += new System.EventHandler(this.addDLLOverrideToolStripMenuItem_MouseHover);
            // 
            // installToGameFolderToolStripMenuItem
            // 
            this.installToGameFolderToolStripMenuItem.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
            this.installToGameFolderToolStripMenuItem.Image = global::DLSSTweaks.ConfigTool.Properties.Resources.install;
            this.installToGameFolderToolStripMenuItem.Name = "installToGameFolderToolStripMenuItem";
            this.installToGameFolderToolStripMenuItem.Size = new System.Drawing.Size(256, 34);
            this.installToGameFolderToolStripMenuItem.Text = "Copy to game folder...";
            this.installToGameFolderToolStripMenuItem.Click += new System.EventHandler(this.installToGameFolderToolStripMenuItem_Click);
            this.installToGameFolderToolStripMenuItem.MouseHover += new System.EventHandler(this.installToGameFolderToolStripMenuItem_MouseHover);
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 42);
            this.splitContainer1.Margin = new System.Windows.Forms.Padding(4);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.lvSettings);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.txtDesc);
            this.splitContainer1.Size = new System.Drawing.Size(856, 1043);
            this.splitContainer1.SplitterDistance = 625;
            this.splitContainer1.TabIndex = 3;
            // 
            // lvSettings
            // 
            this.lvSettings.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.chSetting,
            this.chValue});
            this.lvSettings.ContextMenuStrip = this.menuSettings;
            this.lvSettings.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvSettings.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lvSettings.FullRowSelect = true;
            this.lvSettings.HideSelection = false;
            this.lvSettings.Location = new System.Drawing.Point(0, 0);
            this.lvSettings.Margin = new System.Windows.Forms.Padding(4);
            this.lvSettings.MultiSelect = false;
            this.lvSettings.Name = "lvSettings";
            this.lvSettings.Size = new System.Drawing.Size(856, 625);
            this.lvSettings.TabIndex = 1;
            this.lvSettings.UseCompatibleStateImageBehavior = false;
            this.lvSettings.View = System.Windows.Forms.View.Details;
            this.lvSettings.SelectedIndexChanged += new System.EventHandler(this.lvSettings_SelectedIndexChanged);
            this.lvSettings.MouseMove += new System.Windows.Forms.MouseEventHandler(this.lvSettings_MouseMove);
            // 
            // chSetting
            // 
            this.chSetting.Text = "Setting";
            this.chSetting.Width = 300;
            // 
            // chValue
            // 
            this.chValue.Text = "Value";
            this.chValue.Width = 300;
            // 
            // menuSettings
            // 
            this.menuSettings.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.menuSettings.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ctxRename,
            this.ctxDelete});
            this.menuSettings.Name = "menuSettings";
            this.menuSettings.Size = new System.Drawing.Size(162, 76);
            // 
            // ctxRename
            // 
            this.ctxRename.Name = "ctxRename";
            this.ctxRename.Size = new System.Drawing.Size(161, 36);
            this.ctxRename.Text = "Rename";
            this.ctxRename.Click += new System.EventHandler(this.ctxRename_Click);
            // 
            // ctxDelete
            // 
            this.ctxDelete.Name = "ctxDelete";
            this.ctxDelete.Size = new System.Drawing.Size(161, 36);
            this.ctxDelete.Text = "Delete";
            this.ctxDelete.Click += new System.EventHandler(this.ctxDelete_Click);
            // 
            // txtDesc
            // 
            this.txtDesc.BackColor = System.Drawing.SystemColors.Window;
            this.txtDesc.Dock = System.Windows.Forms.DockStyle.Fill;
            this.txtDesc.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtDesc.Location = new System.Drawing.Point(0, 0);
            this.txtDesc.Margin = new System.Windows.Forms.Padding(4);
            this.txtDesc.Multiline = true;
            this.txtDesc.Name = "txtDesc";
            this.txtDesc.ReadOnly = true;
            this.txtDesc.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtDesc.Size = new System.Drawing.Size(856, 414);
            this.txtDesc.TabIndex = 5;
            // 
            // statusStrip1
            // 
            this.statusStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.lblIniPath});
            this.statusStrip1.Location = new System.Drawing.Point(0, 1085);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Padding = new System.Windows.Forms.Padding(2, 0, 18, 0);
            this.statusStrip1.Size = new System.Drawing.Size(856, 39);
            this.statusStrip1.TabIndex = 4;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // lblIniPath
            // 
            this.lblIniPath.Name = "lblIniPath";
            this.lblIniPath.Size = new System.Drawing.Size(28, 30);
            this.lblIniPath.Text = "...";
            // 
            // Main
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(11F, 24F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(856, 1124);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.menuStrip1);
            this.Controls.Add(this.statusStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "Main";
            this.Text = "DLSSTweaks ConfigTool";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Main_FormClosing);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.menuSettings.ResumeLayout(false);
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ToolStripMenuItem loadToolStripMenuItem;
        private ToolStripMenuItem saveToolStripMenuItem;
        private MenuStrip menuStrip1;
        private ListViewEx lvSettings;
        private ColumnHeader chSetting;
        private ColumnHeader chValue;
        private SplitContainer splitContainer1;
        private TextBox txtDesc;
        private ToolStripMenuItem addDLLOverrideToolStripMenuItem;
        private ToolStripMenuItem installToGameFolderToolStripMenuItem;
        private StatusStrip statusStrip1;
        private ToolStripStatusLabel lblIniPath;
        private ContextMenuStrip menuSettings;
        private ToolStripMenuItem ctxDelete;
        private ToolStripMenuItem ctxRename;
        private ToolTip toolTip;
    }
}