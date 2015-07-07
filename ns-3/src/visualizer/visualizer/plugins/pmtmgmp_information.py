import gtk

import ns.core
import ns.network
import ns.visualizer
import ns.pmtmgmp

from visualizer.base import InformationWindow
from zmq.backend.cython.constants import NULL


class ShowPmtmgmpInforamtion(InformationWindow):
    def __init__(self, visualizer, node_index ,device):
        InformationWindow.__init__(self)
        self.win = gtk.Dialog(parent=visualizer.window,
                              flags=gtk.DIALOG_DESTROY_WITH_PARENT|gtk.DIALOG_NO_SEPARATOR,
                              buttons=(gtk.STOCK_CLOSE, gtk.RESPONSE_CLOSE))
        self.win.set_default_size(gtk.gdk.screen_width()/2, gtk.gdk.screen_height()/2)
        self.win.connect("response", self._response_cb)
        self.win.set_title("PMTMGMP information for node %i" % node_index)
        self.visualizer = visualizer
        self.node_index = node_index
        self.table_model = gtk.ListStore(str, str, int, int, int, float, bool)

        self.pmtmgmp = device.GetRoutingProtocol()

        #Information label of Node
        type_index = self.pmtmgmp.GetNodeType()
        type_str = ""
        if type_index == 1:
            type_str = "Mesh_STA"
        if type_index == 2:
            type_str = "Mesh_Access_Point"
        if type_index == 4:
            type_str = "Mesh_Portal"
        if type_index == 8:
            type_str = "Mesh_Secondary_Point"
        self.label = gtk.Label("Information For Node[" + bytes(self.node_index) + "]:" + type_str)
        self.label.show()

        vbox = gtk.VBox()
        hbox = gtk.HBox()
        hbox.pack_start(self.label, False, False, 3)
        hbox.show()
        vbox.show()
        vbox.pack_start(hbox, False, True, 0)
        h_eparator = gtk.HSeparator()
        h_eparator.show()
        vbox.pack_start(h_eparator, False, True, 3)

        #NoteBook
        self.notebook = gtk.Notebook()
        self.notebook.show()
        vbox.add(self.notebook)

        #ScrolledWindow
        sw = gtk.ScrolledWindow()
        sw.set_properties(hscrollbar_policy=gtk.POLICY_AUTOMATIC,
                          vscrollbar_policy=gtk.POLICY_AUTOMATIC)
        sw.show()
        sw.add_with_viewport(vbox)
        self.win.vbox.add(sw)

        self.visualizer.add_information_window(self)
        self.win.show()

    def _response_cb(self, win, response):
        self.win.destroy()
        self.visualizer.remove_information_window(self)

    def update(self):
        class New_Notebook_Page():
            def __init__(self):
                self.page_vbox = gtk.VBox()
                self.page_vbox.show()
                list_type = []
                self.page_column_num = 0
                for self.page_column_num in range(0, ns.pmtmgmp.my11s.PmtmgmpRoutePath().GetMaxCandidateNum()):
                    list_type.append(str)
                self.page_treeview_model = gtk.ListStore(int, str, str, int, int, float, str, str, *list_type)
                self.page_treeview = gtk.TreeView(self.page_treeview_model)
                self.page_treeview.show()
                self.page_vbox.add(self.page_treeview)
                page_treeviewcolum = gtk.TreeViewColumn(' Index ', gtk.CellRendererText(), text = 0)
                self.page_treeview.append_column(page_treeviewcolum)
                page_treeviewcolum = gtk.TreeViewColumn(' MTERP ', gtk.CellRendererText(), text = 1)
                self.page_treeview.append_column(page_treeviewcolum)
                page_treeviewcolum = gtk.TreeViewColumn(' MSECP ', gtk.CellRendererText(), text = 2)
                self.page_treeview.append_column(page_treeviewcolum)
                page_treeviewcolum = gtk.TreeViewColumn(' GSN ', gtk.CellRendererText(), text = 3)
                self.page_treeview.append_column(page_treeviewcolum)
                page_treeviewcolum = gtk.TreeViewColumn(' HopCount ', gtk.CellRendererText(), text = 4)
                self.page_treeview.append_column(page_treeviewcolum)
                page_treeviewcolum = gtk.TreeViewColumn(' Metric ', gtk.CellRendererText(), text = 5)
                self.page_treeview.append_column(page_treeviewcolum)
                page_treeviewcolum = gtk.TreeViewColumn(' FromNode ', gtk.CellRendererText(), text = 6)
                self.page_treeview.append_column(page_treeviewcolum)
                page_treeviewcolum = gtk.TreeViewColumn(' Status ', gtk.CellRendererText(), text = 7)
                self.page_treeview.append_column(page_treeviewcolum)
                for self.page_column_num in range(0, ns.pmtmgmp.my11s.PmtmgmpRoutePath().GetMaxCandidateNum()):
                    page_treeviewcolum = gtk.TreeViewColumn(' Candidate_ ' +  bytes(self.page_column_num) + ' ', gtk.CellRendererText(), text= self.page_column_num + 8)
                    self.page_treeview.append_column(page_treeviewcolum)

        #Information label of Node
        type_index = self.pmtmgmp.GetNodeType()
        type_str = ""
        if type_index == 0:
            type_str = "Mesh_STA"
        if type_index == 1:
            type_str = "Mesh_Access_Point"
        if type_index == 2:
            type_str = "Mesh_Portal"
        if type_index == 3:
            type_str = "Mesh_Secondary_Point"
        self.label = gtk.Label("Information For Node[" + bytes(self.node_index) + "]:" + type_str)
        self.label.show()

        #Shouw Route Table
        route_table = self.pmtmgmp.GetPmtmgmpRPRouteTable()
        for i in range(0, self.notebook.get_n_pages()):
            self.notebook.remove_page(i)
        if (route_table.GetTableSize() == 0):
            lable_page = gtk.Label("No Path")
            lable_page.show()
            lable_title = gtk.Label("No Path")
            lable_title.show()
            self.notebook.append_page(lable_page, gtk.Label("No Path"))
        else:
            for i in range(0, route_table.GetTableSize()):
                page =  New_Notebook_Page()
                page_treeview_iter = page.page_treeview_model.append()
                self.notebook.append_page(page.page_vbox, gtk.Label("Node 1" + bytes(i)))

def populate_node_menu(viz, node, menu):
    ns3_node = ns.network.NodeList.GetNode(node.node_index)
    WmnDevice = None
    for devI in range(ns3_node.GetNDevices()):
        dev = ns3_node.GetDevice(devI)
        name = ns.core.Names.FindName(dev)
        if dev.GetInstanceTypeId().GetName() == "ns3::WmnPointDevice":
            WmnDevice = dev
    if WmnDevice is None:
        print "No PMTMGMP"
        return

    menu_item = gtk.MenuItem("Show Pmtmgmp Information")
    menu_item.show()

    def _show_pmtmgmp_information(dummy_menu_item):
        ShowPmtmgmpInforamtion(viz, node.node_index, WmnDevice)


    menu_item.connect("activate", _show_pmtmgmp_information)
    menu.add(menu_item)

def register(viz):
    viz.connect("populate-node-menu", populate_node_menu)