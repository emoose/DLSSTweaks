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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Main));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.addDLLOverrideToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.txtDesc = new System.Windows.Forms.TextBox();
            this.loadToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.lvSettings = new DLSSTweaks.ConfigTool.ListViewEx();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.saveToolStripMenuItem,
            this.loadToolStripMenuItem,
            this.addDLLOverrideToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(630, 30);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // addDLLOverrideToolStripMenuItem
            // 
            this.addDLLOverrideToolStripMenuItem.Image = global::DLSSTweaks.ConfigTool.Properties.Resources.dlloverride;
            this.addDLLOverrideToolStripMenuItem.Name = "addDLLOverrideToolStripMenuItem";
            this.addDLLOverrideToolStripMenuItem.Size = new System.Drawing.Size(161, 26);
            this.addDLLOverrideToolStripMenuItem.Text = "Add DLL Override";
            this.addDLLOverrideToolStripMenuItem.Click += new System.EventHandler(this.addDLLOverrideToolStripMenuItem_Click);
            this.addDLLOverrideToolStripMenuItem.MouseHover += new System.EventHandler(this.addDLLOverrideToolStripMenuItem_MouseHover);
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 30);
            this.splitContainer1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
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
            this.splitContainer1.Size = new System.Drawing.Size(630, 720);
            this.splitContainer1.SplitterDistance = 434;
            this.splitContainer1.SplitterWidth = 3;
            this.splitContainer1.TabIndex = 3;
            // 
            // txtDesc
            // 
            this.txtDesc.Dock = System.Windows.Forms.DockStyle.Fill;
            this.txtDesc.Location = new System.Drawing.Point(0, 0);
            this.txtDesc.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.txtDesc.Multiline = true;
            this.txtDesc.Name = "txtDesc";
            this.txtDesc.ReadOnly = true;
            this.txtDesc.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtDesc.Size = new System.Drawing.Size(630, 283);
            this.txtDesc.TabIndex = 5;
            // 
            // loadToolStripMenuItem
            // 
            this.loadToolStripMenuItem.Image = global::DLSSTweaks.ConfigTool.Properties.Resources.refresh;
            this.loadToolStripMenuItem.Name = "loadToolStripMenuItem";
            this.loadToolStripMenuItem.Size = new System.Drawing.Size(90, 26);
            this.loadToolStripMenuItem.Text = "Reload";
            this.loadToolStripMenuItem.Click += new System.EventHandler(this.loadToolStripMenuItem_Click);
            this.loadToolStripMenuItem.MouseHover += new System.EventHandler(this.loadToolStripMenuItem_MouseHover);
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Image = global::DLSSTweaks.ConfigTool.Properties.Resources.save;
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(74, 26);
            this.saveToolStripMenuItem.Text = "Save";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
            this.saveToolStripMenuItem.MouseHover += new System.EventHandler(this.saveToolStripMenuItem_MouseHover);
            // 
            // lvSettings
            // 
            this.lvSettings.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.lvSettings.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvSettings.FullRowSelect = true;
            this.lvSettings.HideSelection = false;
            this.lvSettings.Location = new System.Drawing.Point(0, 0);
            this.lvSettings.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.lvSettings.MultiSelect = false;
            this.lvSettings.Name = "lvSettings";
            this.lvSettings.Size = new System.Drawing.Size(630, 434);
            this.lvSettings.TabIndex = 1;
            this.lvSettings.UseCompatibleStateImageBehavior = false;
            this.lvSettings.View = System.Windows.Forms.View.Details;
            this.lvSettings.ItemMouseHover += new System.Windows.Forms.ListViewItemMouseHoverEventHandler(this.lvSettings_ItemMouseHover);
            this.lvSettings.SelectedIndexChanged += new System.EventHandler(this.lvSettings_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Setting";
            this.columnHeader1.Width = 200;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Value";
            this.columnHeader2.Width = 250;
            // 
            // Main
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(630, 750);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.menuStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.Name = "Main";
            this.Text = "DLSSTweaks ConfigTool";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

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