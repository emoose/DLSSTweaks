/* ListViewEx.cs 
 * This file contains the definition of the class ListViewEx, which is a
 * reusable class derived from ListView. 
*/

#region Namespaces
using System;
using System.Drawing;
using System.Collections;
using System.Diagnostics;
using System.Windows.Forms;
using System.ComponentModel;
using System.Collections.Specialized;
using System.Runtime.InteropServices;
#endregion

namespace DLSSTweaks.ConfigTool
{
    /// <summary>
    /// Class derived from ListView to give ability to display controls
    /// like TextBox and Combobox
    /// </summary>
    public class ListViewEx : ListView
    {
        public event EventHandler ValueChanged;

        #region The RECT structure
        /// <summary>
        /// This struct type will be used as the oupput 
        /// param of the SendMessage( GetSubItemRect ). 
        /// Actually it is a representation for the strucure 
        /// RECT in Win32
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        internal struct RECT
        {
            public int left;
            public int top;
            public int right;
            public int bottom;
        }
        #endregion

        #region Win32 Class
        /// <summary>
        /// Summary description for Win32.
        /// </summary>
        internal class Win32
        {
            /// <summary>
            /// This is the number of the message for getting the sub item rect.
            /// </summary>
            public const int LVM_GETSUBITEMRECT = (0x1000) + 56;

            /// <summary>
            /// As we are using the detailed view for the list,
            /// LVIR_BOUNDS is the best parameters for RECT's 'left' member.
            /// </summary>
            public const int LVIR_BOUNDS = 0;

            /// <summary>
            /// Sending message to Win32
            /// </summary>
            /// <param name="hWnd">Handle to the control</param>
            /// <param name="messageID">ID of the message</param>
            /// <param name="wParam"></param>
            /// <param name="lParam"></param>
            /// <returns></returns>
            [DllImport("user32.dll", SetLastError = true)]
            public static extern int SendMessage(IntPtr hWnd, int messageID, int wParam, ref RECT lParam);
        }
        #endregion

        #region SubItem Class
        /// <summary>
        /// This class is used to represent 
        /// a listview subitem.
        /// </summary>
        internal class SubItem
        {
            /// <summary>
            /// Item index
            /// </summary>
            public readonly int row;

            /// <summary>
            /// Subitem index
            /// </summary>
            public readonly int col;

            /// <summary>
            /// Parameterized contructor
            /// </summary>
            /// <param name="row"></param>
            /// <param name="col"></param>
            public SubItem(int row, int col)
            {
                this.row = row;
                this.col = col;
            }
        }
        #endregion

        #region Variables & Properties
        /// <summary>
        /// If this variable is true, then 
        /// subitems for an item is added 
        /// automatically, if not present.
        /// </summary>
        private bool addSubItem = false;
        public bool AddSubItem
        {
            set
            {
                this.addSubItem = value;
            }
        }

        /// <summary>
        /// This variable tells whether the combo box 
        /// is needed to be displayed after its selection
        /// is changed
        /// </summary>
        private bool hideComboAfterSelChange = false;
        public bool HideComboAfterSelChange
        {
            set
            {
                this.hideComboAfterSelChange = value;
            }
        }

        /// <summary>
        /// Represents current row
        /// </summary>
        private int row = -1;

        /// <summary>
        /// Represents current column
        /// </summary>
        private int col = -1;

        /// <summary>
        /// Textbox to display in the editable cells
        /// </summary>
        private TextBox textBox = new TextBox();

        /// <summary>
        /// Combo box to display in the associated cells
        /// </summary>
        private ComboBox combo = new ComboBox();

        /// <summary>
        /// This is a flag variable. This is used to determine whether
        /// Mousebutton is pressed within the listview
        /// </summary>
        private bool mouseDown = false;

        /// <summary>
        /// To store, subitems that contains comboboxes and text boxes
        /// </summary>
        private Hashtable customCells = new Hashtable();
        #endregion

        #region Methods
        /// <summary>
        /// Constructor
        /// </summary>
        public ListViewEx()
        {
            // Initialize controls
            this.InitializeComponent();
        }

        /// <summary>
        /// Initializes the text box and combo box
        /// </summary>
        private void InitializeComponent()
        {
            // Text box
            this.textBox.Visible = false;
            textBox.BorderStyle = BorderStyle.FixedSingle;
            this.textBox.Leave += new EventHandler(textBox_Leave);
            this.textBox.KeyPress += new KeyPressEventHandler(TextBox_KeyPress);

            // Combo box
            this.combo.Visible = false;
            this.Controls.Add(this.textBox);
            this.Controls.Add(this.combo);
            this.combo.DropDownStyle = ComboBoxStyle.DropDownList;
            this.combo.SelectedIndexChanged += new EventHandler(combo_SelectedIndexChanged);
        }

        private void TextBox_KeyPress(object? sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 13)
                textBox.Parent.Focus();
        }

        /// <summary>
        /// This method will send LVM_GETSUBITEMRECT message to 
        /// get the current subitem bouds of the listview
        /// </summary>
        /// <param name="clickPoint"></param>
        /// <returns></returns>
        private RECT GetSubItemRect(Point clickPoint)
        {
            // Create output param
            RECT subItemRect = new RECT();

            // Reset the indices
            this.row = this.col = -1;

            // Check whether there is any item at the mouse point
            ListViewItem item = this.GetItemAt(clickPoint.X, clickPoint.Y);

            if (item != null)
            {
                for (int index = 0; index < this.Columns.Count; index++)
                {
                    // We need to pass the 1 based index of the subitem.
                    subItemRect.top = index + 1;

                    // To get the boudning rectangle, as we are using the report view
                    subItemRect.left = Win32.LVIR_BOUNDS;
                    try
                    {
                        // Send Win32 message for getting the subitem rect. 
                        // result = 0 means error occuured
                        int result = Win32.SendMessage(this.Handle, Win32.LVM_GETSUBITEMRECT, item.Index, ref subItemRect);
                        if (result != 0)
                        {
                            // This case happens when items in the first columnis selected.
                            // So we need to set the column number explicitly
                            if (clickPoint.X < subItemRect.left)
                            {
                                this.row = item.Index;
                                this.col = 0;
                                break;
                            }
                            if (clickPoint.X >= subItemRect.left & clickPoint.X <=
                                subItemRect.right)
                            {
                                this.row = item.Index;
                                // Add 1 because of the presence of above condition
                                this.col = index + 1;
                                break;
                            }
                        }
                        else
                        {
                            // This call will create a new Win32Exception with the last Win32 Error.
                            throw new Win32Exception();
                        }
                    }
                    catch (Win32Exception ex)
                    {
                        Trace.WriteLine(string.Format("Exception while getting subitem rect, {0}", ex.Message));
                    }
                }
            }
            return subItemRect;
        }

        /// <summary>
        /// Set a text box in a cell
        /// </summary>
        /// <param name="row">The 0-based index of the item.  Give -1 if you
        ///					  want to set a text box for every items for a
        ///					  given "col" variable.
        ///	</param>
        /// <param name="col">The 0-based index of the column. Give -1 if you
        ///					  want to set a text box for every subitems for a
        ///					  given "row" variable.
        ///	</param>
        public void AddEditableCell(int row, int col)
        {
            // Add the cell into the hashtable
            // Value is setting as null because it is an editable cell
            this.customCells[new SubItem(row, col)] = null;
        }

        /// <summary>
        /// Set a combobox in a cell
        /// </summary>
        /// <param name="row"> The 0-based index of the item.  Give -1 if you
        ///					   want to set a combo box for every items for a
        ///					   given "col" variable.
        ///	</param>
        /// <param name="col"> The 0-based index of the column. Give -1 if you
        ///					   want to set a combo box for every subitems for a
        ///					   given "row" variable.
        ///	</param>
        /// <param name="data"> Items of the combobox 
        /// </param>
        public void AddComboBoxCell(int row, int col, StringCollection data)
        {
            // Add the cell into the hashtable
            // Value for the hashtable is the combobox items
            this.customCells[new SubItem(row, col)] = data;
        }

        /// <summary>
        /// Set a combobox in a cell
        /// </summary>
        /// <param name="row"> The 0-based index of the item.  Give -1 if you
        ///					   want to set a combo box for every items for a
        ///					   given "col" variable.
        ///	</param>
        /// <param name="col"> The 0-based index of the column. Give -1 if you
        ///					   want to set a combo box for every subitems for a
        ///					   given "row" variable.
        ///	</param>
        /// <param name="data"> Items of the combobox 
        /// </param>
        public void AddComboBoxCell(int row, int col, string[] data)
        {
            try
            {
                StringCollection param = new StringCollection();
                param.AddRange(data);
                this.AddComboBoxCell(row, col, param);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
        }

        /// <summary>
        /// This method will display the combobox
        /// </summary>
        /// <param name="location">Location of the combobox</param>
        /// <param name="sz">Size of the combobox</param>
        /// <param name="data">Combobox items</param>
        private void ShowComboBox(Point location, Size sz, StringCollection data)
        {
            try
            {
                // Initialize the combobox
                combo.Size = sz;
                combo.Location = location;
                // Add items
                combo.Items.Clear();
                foreach (string text in data)
                {
                    combo.Items.Add(text);
                }
                // Set the current text, take it from the current listview cell
                combo.Text = this.Items[row].SubItems[col].Text;
                // Calculate and set drop down width
                combo.DropDownWidth = this.GetDropDownWidth(data);
                // Show the combo
                combo.Show();
            }
            catch (ArgumentOutOfRangeException)
            {
                // Sink
            }
        }

        /// <summary>
        /// This method will display the textbox
        /// </summary>
        /// <param name="location">Location of the textbox</param>
        /// <param name="sz">Size of the textbox</param>
        private void ShowTextBox(Point location, Size sz)
        {
            try
            {
                // Initialize the textbox
                textBox.Size = sz;
                textBox.Location = location;
                // Set text, take it from the current listview cell
                textBox.Text = this.Items[row].SubItems[col].Text;
                // Shopw the text box
                textBox.Show();
                textBox.Focus();
            }
            catch (ArgumentOutOfRangeException)
            {
                // Sink
            }
        }

        protected override void OnColumnWidthChanging(ColumnWidthChangingEventArgs e)
        {
            base.OnColumnWidthChanging(e);

            OnMouseUp(null);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseUp(MouseEventArgs e)
        {
            try
            {
                // Hide the controls
                this.textBox.Visible = this.combo.Visible = false;

                // If no mouse down happned in this listview, 
                // no need to show anything
                if (!mouseDown)
                {
                    return;
                }

                // The listview should be having the following properties enabled
                // 1. FullRowSelect = true
                // 2. View should be Detail;
                if (!this.FullRowSelect || this.View != View.Details)
                {
                    return;
                }

                // Reset the mouse down flag
                mouseDown = false;

                // Get the subitem rect at the mouse point.
                // Remeber that the current row index and column index will also be
                // Modified within the same method
                RECT rect = this.GetSubItemRect(new Point(e.X, e.Y));

                // If the above method is executed with any error,
                // The row index and column index will be -1;
                if (this.row != -1 && this.col != -1)
                {
                    // Check whether combobox or text box is set for the current cell
                    SubItem cell = GetKey(new SubItem(this.row, this.col));

                    if (cell != null)
                    {
                        // Set the size of the control(combobox/editbox)
                        // This should be composed of the height of the current items and
                        // width of the current column
                        Size sz = new Size(this.Columns[col].Width, Items[row].Bounds.Height);

                        // Determine the location where the control(combobox/editbox) to be placed
                        Point location = col == 0 ? new Point(0, rect.top) : new Point(rect.left, rect.top);

                        ValidateAndAddSubItems();

                        // Decide which conrol to be displayed.
                        if (this.customCells[cell] == null)
                        {
                            this.ShowTextBox(location, sz);
                        }
                        else
                        {
                            this.ShowComboBox(location, sz, (StringCollection)this.customCells[cell]);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
        }

        /// <summary>
        /// 
        /// </summary>
        private void ValidateAndAddSubItems()
        {
            try
            {
                while (this.Items[this.row].SubItems.Count < this.Columns.Count && this.addSubItem)
                {
                    this.Items[this.row].SubItems.Add("");
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
        }

        /// <summary>
        /// This message will get the largest text from the given
        /// stringarray, and will calculate the width of a control which 
        /// will contain that text.
        /// </summary>
        /// <param name="data"></param>
        /// <returns></returns>
        private int GetDropDownWidth(StringCollection data)
        {
            // If array is empty just return the combo box width
            if (data.Count == 0)
            {
                return this.combo.Width;
            }

            // Set the first text as the largest
            string maximum = data[0];

            // Now iterate thru each string, to findout the
            // largest
            foreach (string text in data)
            {
                if (maximum.Length < text.Length)
                {
                    maximum = text;
                }
            }
            // Calculate and return the width .
            return (int)(this.CreateGraphics().MeasureString(maximum, this.Font).Width);
        }

        /// <summary>
        /// For this method, we will get a Subitem. 
        /// Then we will iterate thru each of the keys and will 
        /// check whther any key contains the given cells row/column.
        /// If it is not found we will check for -1 in any one
        /// </summary>
        /// <param name="cell"></param>
        /// <returns></returns>
        private SubItem GetKey(SubItem cell)
        {
            try
            {
                foreach (SubItem key in this.customCells.Keys)
                {
                    // Case 1: Any particular cells is  enabled for a control(Textbox/combobox)
                    if (key.row == cell.row && key.col == cell.col)
                    {
                        return key;
                    }
                    // Case 2: Any particular column is  enabled for a control(Textbox/combobox)
                    else if (key.row == -1 && key.col == cell.col)
                    {
                        return key;
                    }
                    // Entire col for a row is is  enabled for a control(Textbox/combobox)
                    else if (key.row == cell.row && key.col == -1)
                    {
                        return key;
                    }
                    // All cells are enabled for a control(Textbox/combobox)
                    else if (key.row == -1 && key.col == -1)
                    {
                        return key;
                    }
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
            return null;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseDown(MouseEventArgs e)
        {
            try
            {
                // Mouse down happened inside listview
                mouseDown = true;

                // Hide the controls
                this.textBox.Hide();
                this.combo.Hide();
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
        }

        /// <summary>
        /// This event handler wll set the current text in the textbox
        /// as the listview's current cell's text, while the textbox 
        /// focus is lost
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void textBox_Leave(object sender, EventArgs e)
        {
            try
            {
                if (this.row != -1 && this.col != -1)
                {
                    bool changed = this.Items[row].SubItems[col].Text != this.textBox.Text;
                    this.Items[row].SubItems[col].Text = this.textBox.Text;
                    this.textBox.Hide();

                    if (changed && this.ValueChanged != null)
                        ValueChanged(this, new EventArgs());
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
        }

        /// <summary>
        /// This event handler wll set the current text in the combobox
        /// as the listview's current cell's text, while the combobox 
        /// selection is changed
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void combo_SelectedIndexChanged(object sender, EventArgs e)
        {
            try
            {
                if (this.row != -1 && this.col != -1)
                {
                    bool changed = this.Items[row].SubItems[col].Text != this.combo.Text;
                    this.Items[row].SubItems[col].Text = this.combo.Text;
                    this.combo.Visible = !this.hideComboAfterSelChange;

                    if (changed && this.ValueChanged != null)
                        ValueChanged(this, new EventArgs());
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.ToString());
            }
        }
        #endregion
    }
}