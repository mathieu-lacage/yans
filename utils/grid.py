#!/usr/bin/env python

import cairo
import sys
import re
import gtk


class DataRange:
    def __init__ (self):
        self.start = 0
        self.end = 0
        self.name = ''

class Event:
    def __init (self, name = '', at = 0):
        self.name = name
        self.at = at

class Line:
    def __init__(self, name):
        self.ranges = []
        self.events = []
        self.name = name
    def __search (self, key):
        l = 0
        u = len (self.ranges)-1
        while l <= u:
            i = int ((l+u)/2)
            if key >= self.ranges[i].start and key <= self.ranges[i].end:
                return i
            elif key < self.ranges[i].start:
                u = i - 1
            else:
                # key > self.ranges[i].end
                l = i + 1
        return -1
    def get_range (self, start, end):
        s = self.__search (start)
        e = self.__search (end)
        if s == -1 and e == -1:
            return []
        elif s == -1:
            return self.ranges[0:e+1]
        elif e == -1:
            return self.ranges[s:len (self.ranges)]
        else:
            return self.ranges[s:e+1]
    def add_range (self, range):
        self.ranges.append (range)
    def add_event (self, event):
        self.events.append (event)
    def sort (self):
        self.events.sort ()
        self.ranges.sort ()
    def get_bounds (self):
        if len (self.events) > 0:
            ev_lo = self.events[0].at
            ev_hi = self.events[-1].at
            if len (self.ranges) > 0:
                ran_lo = self.ranges.data[0].start
                ran_hi = self.ranges.data[len (self.ranges)-1].end
                return (min (ev_lo, ran_lo), max (ev_hi, ran_hi))
            else:
                return (ev_lo, ev_hi)
        else:
            if len (self.ranges) > 0:
                lo = self.ranges[0].start
                hi = self.ranges[len (self.ranges)-1].end
                return (lo, hi)
            else:
                return (0,0)



class Color:
    def __init__ (self, r = 0.0, g = 0.0, b = 0.0):
        self.r = r
        self.g = g
        self.b = b
    def set (self, r, g, b):
        self.r = r
        self.g = g
        self.b = b

class Colors:
    # XXX add more
    default_colors = [Color (1,0,0), Color (0,1,0), Color (0,0,1),Color (1,1,0), Color(1,0,1), Color (0,1,1)]
    def __init__ (self):
        self.__colors = {}
    def add (self, name, color):
        self.__colors[name] = color
    def lookup (self, name):
        if not self.__colors.has_key (name):
            self.add (name, self.default_colors.pop ())
        return self.__colors.get(name)


class TopLegendRenderer:
    def __init__ (self):
        self.__padding = 10
    def set_padding (self, padding):
        self.__padding = padding
    def set_legends (self, legends, colors):
        self.__legends = legends
        self.__colors = colors
    def layout (self, width):
        self.__width = width
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 1,1)
        ctx = cairo.Context(surface)
        line_height = 0
        total_height = self.__padding
        line_used = self.__padding
        for legend in self.__legends:
            (t_width, t_height) = ctx.text_extents (legend)[2:4]
            item_width = self.__padding +  self.__padding + t_width +  self.__padding
            item_height = t_height + self.__padding
            if item_height > line_height:
                line_height = item_height
            if line_used + item_width > self.__width:
                line_used = self.__padding + item_width
                total_height += line_height
            else:
                line_used += item_width
            x = line_used - item_width
        total_height += line_height
        self.__height = total_height
            
    def get_height (self):
        return self.__height
    def draw (self, ctx):
        i = 0
        line_height = 0
        total_height = self.__padding
        line_used = self.__padding
        for legend in self.__legends:
            (t_width, t_height) = ctx.text_extents (legend)[2:4]
            item_width = self.__padding +  self.__padding + t_width +  self.__padding
            item_height = t_height + self.__padding
            if item_height > line_height:
                line_height = item_height
            if line_used + item_width > self.__width:
                line_used = self.__padding + item_width
                total_height += line_height
            else:
                line_used += item_width
            x = line_used - item_width
            ctx.rectangle (x, total_height, self.__padding, self.__padding)
            ctx.set_source_rgb (0,0,0)
            ctx.set_line_width (2)
            ctx.stroke_preserve ()
            ctx.set_source_rgb (self.__colors[i].r,
                                self.__colors[i].g,
                                self.__colors[i].b)
            ctx.fill ()
            ctx.move_to (x+self.__padding*2, total_height+t_height)
            ctx.set_source_rgb (0,0,0)
            ctx.show_text (legend)
            i += 1

        return

class DataRenderer:
    def __init__ (self):
        self.__top_border = 10
        self.__bot_border = 0
        self.__side_border = 10
        self.__padding = 10
        self.__data_height = 10
    def set_range (self, start, end):
        self.__start = start
        self.__end = end
    def set_data (self, data, colors):
        self.__lines = data
        self.__colors = colors
    def set_data_height (self, height):
        self.__data_height = height
    def set_padding (self, padding):
        self.__padding = padding
    def set_border (self, top, bottom, side):
        self.__top_border = top
        self.__bot_border = bottom
        self.__side_border = side
    def get_left_width (self):
        return self.__left_width
    def get_height (self):
        return self.__height
    def layout (self, width):
        self.__width = width
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 1,1)
        ctx = cairo.Context(surface)
        max_width = 0
        total_height =  0
        names = {}
        for line in self.__lines:
            (width, height) = ctx.text_extents (line.name)[2:4]
            if width > max_width:
                max_width = width
            total_height += height
            total_height += self.__padding
        self.__left_width = max_width + self.__side_border * 2
        self.__height = total_height + self.__top_border + self.__bot_border

    def draw (self, ctx):
        # right justify the labels
        ctx.save ()
        y = self.__top_border
        graph_width = self.__width - self.__left_width
        for line in self.__lines:
            (t_y_advance, t_width, t_height) = ctx.text_extents (line.name)[1:4]
            ctx.move_to (self.__left_width - self.__side_border - t_width, y-t_y_advance)
            ctx.set_source_rgb (0,0,0)
            ctx.show_text (line.name)
            for data_range in line.get_range (self.__start, self.__end):
                #print "draw s="+str (data_range.start)+", e="+str (data_range.end)
                current_start = max (data_range.start, self.__start)
                current_end = min (data_range.end, self.__end)
                x_start = self.__left_width + (current_start - self.__start) * graph_width/ (self.__end - self.__start)
                x_end = self.__left_width + (current_end - self.__start) * graph_width / (self.__end - self.__start)
                if (x_end - x_start) > 1:
                    ctx.rectangle (x_start, y, x_end - x_start, self.__data_height)
                    ctx.set_line_width (1)
                    ctx.set_source_rgb (0,0,0)
                    ctx.stroke_preserve ()
                    color = self.__colors.lookup (data_range.name)
                    ctx.set_source_rgb (color.r, color.g, color.b)
                    ctx.fill ()
            y += self.__padding
            y += max (t_height, self.__data_height)
        ctx.move_to (self.__left_width, 0)
        ctx.line_to (self.__left_width, self.__height)
        ctx.close_path ()
        ctx.set_line_width (2)
        ctx.set_source_rgb (0,0,0)
        ctx.stroke ()
        ctx.restore ()
        return

class ScaleRenderer:
    def __init__ (self):
        self.__top = 0
        return
    def set_bounds (self, lo, hi):
        self.__lo = lo
        self.__hi = hi
    def get_position (self, x):
        real_x = (x - self.__lo ) * self.__width / (self.__hi - self.__lo)
        return real_x
    def set_top (self):
        self.__top = 1
    def set_bot (self):
        self.__top = 0
    def layout (self, width):
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 1,1)
        ctx = cairo.Context(surface)

        # calculate scale delta
        data_delta = self.__hi - self.__lo
        closest = 1
        while (closest*10) < data_delta:
            closest *= 10
        if (data_delta / closest) == 0:
            delta = closest
        elif (data_delta / closest) == 1:
            delta = closest / 10
        else:
            delta = closest
        start = self.__lo - (self.__lo % delta) + delta
        end = self.__hi - (self.__hi % delta)

        self.__delta = delta
        self.__width = width

        # calculate text height
        max_text_height = 0
        ticks = range (int (start), int (end + delta), int (delta))
        for x in ticks:
            (t_height) = ctx.text_extents (str (x))[3]
            if t_height > max_text_height:
                max_text_height = t_height
                
        height = max_text_height + 10
        self.__height = height
    
    def get_height (self):
        return self.__height
    def draw (self, ctx):
        delta = self.__delta
        start = self.__lo - (self.__lo % delta) + delta
        end = self.__hi - (self.__hi % delta)

        if self.__top == 1:
            s = -1
        else:
            s = 1
        # print scale points
        ctx.set_source_rgb (0, 0, 0)
        ctx.set_line_width (1.0)
        ticks = range (int (start), int (end + delta), int (delta))
        for x in ticks:
            real_x = (x - self.__lo ) * self.__width / (self.__hi - self.__lo)
            ctx.move_to (real_x, 0)
            ctx.line_to (real_x, 5*s)
            ctx.close_path ()
            ctx.stroke ()
            (t_y_bearing, t_width, t_height) = ctx.text_extents (str (x))[1:4]
            if self.__top:
                text_delta = t_height + t_y_bearing
            else:
                text_delta = -t_y_bearing
            ctx.move_to (real_x - t_width/2, (5 + 5 + text_delta)*s)
            ctx.show_text (str (x))
        # draw subticks
        delta /= 10
        if delta > 0:
            start = self.__lo - (self.__lo % delta) + delta
            end = self.__hi - (self.__hi % delta)
            for x in range (int (start), int (end + delta), int (delta)):
                real_x = (x - self.__lo ) * self.__width / (self.__hi - self.__lo)
                ctx.move_to (real_x, 0)
                ctx.line_to (real_x, 3*s)
                ctx.close_path ()
                ctx.stroke ()
        
        

class GraphicRenderer:
    def __init__(self, start, end):
        self.__start = float (start)
        self.__end = float (end)
        self.__mid_scale = ScaleRenderer ()
        self.__mid_scale.set_top ()
        self.__bot_scale = ScaleRenderer ()
        self.__bot_scale.set_bounds (start, end)
        self.__bot_scale.set_bot ()
        self.__width = 1
        self.__height = 1
    def get_width (self):
        return self.__width
    def get_height (self):
        return self.__height
    # return x, y, width, height
    def get_data_rectangle (self):
        y_start = self.__top_legend.get_height ()
        x_start = self.__data.get_left_width ()
        return (x_start, y_start, self.__width - x_start, self.__data.get_height ())
    def scale_data (self, x):
        x_start = self.__data.get_left_width ()
        x_scaled = x / (self.__width - x_start) * (self.__r_end - self.__r_start)
        return x_scaled
    # return x, y, width, height
    def get_selection_rectangle (self):
        y_start = self.__top_legend.get_height () + self.__data.get_height () + self.__mid_scale.get_height () + 20
        y_height = self.__bot_scale.get_height () + 20
        x_start = self.__bot_scale.get_position (self.__r_start)
        x_end = self.__bot_scale.get_position (self.__r_end)
        return (x_start,y_start,x_end-x_start,y_height)
    def scale_selection (self, x):
        x_scaled = x / self.__width * (self.__end - self.__start)
        return x_scaled
    def set_range (self,start, end):
        s = min (start, end)
        e = max (start, end)
        start = max (self.__start, s)
        end = min (self.__end, e)
        self.__r_start = start
        self.__r_end = end
        self.__data.set_range (start, end)
        self.__mid_scale.set_bounds (start, end)
        self.layout (self.__width, self.__height)
    def get_range (self):
        return (self.__r_start, self.__r_end)
    def set_data (self, data):
        self.__data = data
    def set_top_legend (self, top_legend):
        self.__top_legend = top_legend
    def layout (self, width, height):
        self.__width = width
        self.__height = height
        self.__top_legend.layout (width)
        top_legend_height = self.__top_legend.get_height ()
        self.__data.layout (width)
        self.__mid_scale.layout (width - self.__data.get_left_width ())
        self.__bot_scale.layout (width)
        return
    def __x_pixel (self, x, width):
        new_x = (x - self.__start) * width / (self.__end - self.__start)
        return new_x
    
    def draw (self, ctx):
        # default background is white
        ctx.save ()
        ctx.set_source_rgb (1, 1, 1)
        ctx.set_operator (cairo.OPERATOR_SOURCE)
        ctx.rectangle (0,0,self.__width,self.__height)
        ctx.fill ()

        # top legend 
        ctx.save ()
        self.__top_legend.draw (ctx)
        top_legend_height = self.__top_legend.get_height ()
        ctx.restore ()

        # separation line
        ctx.move_to (0, top_legend_height)
        ctx.line_to (self.__width, top_legend_height)
        ctx.close_path ()
        ctx.set_line_width (2)
        ctx.set_source_rgb (0,0,0)
        ctx.stroke ()

        # data
        ctx.save ()
        ctx.translate (0,
                       top_legend_height)
        self.__data.draw (ctx)
        ctx.restore ()

        # scale below data
        ctx.save ()
        ctx.translate (self.__data.get_left_width (),
                       top_legend_height + self.__data.get_height () + self.__mid_scale.get_height ())
        self.__mid_scale.draw (ctx)
        ctx.restore ()

        height_used = top_legend_height + self.__data.get_height () + self.__mid_scale.get_height ()

        # separation between scale and left pane
        ctx.move_to (self.__data.get_left_width (), height_used)
        ctx.rel_line_to (0, -self.__mid_scale.get_height ())
        ctx.close_path ()
        ctx.set_source_rgb (0,0,0)
        ctx.set_line_width (2)
        ctx.stroke ()

        # separation below scale
        ctx.move_to (0, height_used)
        ctx.line_to (self.__width, height_used)
        ctx.close_path ()
        ctx.set_line_width (2)
        ctx.set_source_rgb (0,0,0)
        ctx.stroke ()

        select_start = self.__bot_scale.get_position (self.__r_start)
        select_end = self.__bot_scale.get_position (self.__r_end)

        # left connection between top scale and bottom scale
        ctx.move_to (0, height_used);
        ctx.line_to (self.__data.get_left_width (), height_used)
        ctx.line_to (select_start, height_used + 20)
        ctx.line_to (0, height_used + 20)
        ctx.line_to (0,height_used)
        ctx.set_source_rgb (0,0,0)
        ctx.set_line_width (1)
        ctx.stroke_preserve ()
        ctx.set_source_rgb (0.9,0.9,0.9)
        ctx.fill ()

        # right connection between top scale and bottom scale
        ctx.move_to (self.__width, height_used)
        ctx.line_to (self.__width, height_used+20)
        ctx.line_to (select_end, height_used+20)
        ctx.line_to (self.__width, height_used)
        ctx.set_source_rgb (0,0,0)
        ctx.set_line_width (1)
        ctx.stroke_preserve ()
        ctx.set_source_rgb (0.9,0.9,0.9)
        ctx.fill ()

        height_used += 20

        # unused area background
        unused_start = self.__bot_scale.get_position (self.__r_start)
        unused_end = self.__bot_scale.get_position (self.__r_end)
        unused_height = self.__bot_scale.get_height () + 20
        ctx.rectangle (0, height_used,
                       unused_start,
                       unused_height)
        ctx.rectangle (unused_end,
                       height_used,
                       self.__width - unused_end,
                       unused_height)
        ctx.set_source_rgb (0.9,0.9,0.9)
        ctx.fill ()        

        # border line around bottom scale
        ctx.move_to (unused_end, height_used)
        ctx.line_to (self.__width, height_used)
        ctx.line_to (self.__width, height_used + unused_height)
        ctx.line_to (0, height_used + unused_height)
        ctx.line_to (0, height_used)
        ctx.line_to (unused_start, height_used)
        ctx.close_path ()
        ctx.set_line_width (2)
        ctx.set_source_rgb (0,0,0)
        ctx.stroke ()
        ctx.move_to (unused_start, height_used)
        ctx.line_to (unused_end, height_used)
        ctx.close_path ()
        ctx.set_line_width (1)
        ctx.set_source_rgb (0.9,0.9,0.9)
        ctx.stroke ()

        # unused area dot borders
        ctx.save ()
        ctx.move_to (max (unused_start, 2), height_used)
        ctx.rel_line_to (0,unused_height)
        ctx.move_to (min (unused_end, self.__width-2), height_used)
        ctx.rel_line_to (0, unused_height)
        ctx.set_dash ([5], 0)
        ctx.set_source_rgb (0,0,0)
        ctx.set_line_width (1)
        ctx.stroke ()
        ctx.restore ()

        # bottom scale
        ctx.save ()
        ctx.translate (0, height_used)
        self.__bot_scale.draw (ctx)
        ctx.restore ()

class GtkGraphicRenderer (gtk.DrawingArea):
    def __init__ (self, data):
        super (GtkGraphicRenderer, self).__init__ ()
        self.__data = data
        self.__moving_left = False
        self.__moving_right = False
        self.__moving_both = False
        self.__moving_top = False
        self.add_events (gtk.gdk.POINTER_MOTION_MASK)
        self.add_events (gtk.gdk.BUTTON_PRESS_MASK)
        self.add_events (gtk.gdk.BUTTON_RELEASE_MASK) 
        self.connect ("expose_event", self.expose)
        self.connect ('size-allocate', self.size_allocate)
        self.connect ('motion-notify-event', self.motion_notify)
        self.connect ('button-press-event', self.button_press)
        self.connect ('button-release-event', self.button_release)
    def set_smaller_zoom (self):
        (start, end) = self.__data.get_range ()
        self.__data.set_range (start, start+(end-start)*10)
        self.__force_full_redraw ()
    def set_bigger_zoom (self):
        (start, end) = self.__data.get_range ()
        self.__data.set_range (start, start+(end-start)/10)
        self.__force_full_redraw ()
    def set_biggest_zoom (self):
        (start, end) = self.__data.get_range ()
        self.__data.set_range (start, start+min (start+10, end))
        self.__force_full_redraw ()
    def output_png (self, filename):
        self.__force_full_redraw ()
        self.__data_buffer.write_to_png(filename)
    def button_press (self, widget, event):
        (x, y, width, height) = self.__data.get_selection_rectangle ()
        (d_x, d_y, d_width, d_height) = self.__data.get_data_rectangle ()
        if event.y > y and event.y < y+height:
            if abs (event.x - x) < 5:
                self.__moving_left = True
                return True
            if abs (event.x - (x+width)) < 5:
                self.__moving_right = True
                return True
            if event.x > x and event.x < x+width:
                self.__moving_both = True
                self.__moving_both_start = event.x
                self.__moving_both_cur = event.x
                return True
        if event.y > d_y and event.y < (d_y + d_height):
            if event.x > d_x and event.x < (d_x + d_width):
                self.__moving_top = True
                self.__moving_top_start = event.x
                self.__moving_top_cur = event.x
                return True
        return False
    def button_release (self, widget, event):
        if self.__moving_left:
            self.__moving_left = False
            left = self.__data.scale_selection (self.__moving_left_cur)
            right = self.__data.get_range ()[1]
            self.__data.set_range (left, right)
            self.__force_full_redraw ()
            return True
        if self.__moving_right:
            self.__moving_right = False
            right = self.__data.scale_selection (self.__moving_right_cur)
            left = self.__data.get_range ()[0]
            self.__data.set_range (left, right)
            self.__force_full_redraw ()
            return True
        if self.__moving_both:
            self.__moving_both = False
            delta = self.__data.scale_selection (self.__moving_both_cur - self.__moving_both_start)
            (left, right) = self.__data.get_range ()
            self.__data.set_range (left+delta, right+delta)
            self.__force_full_redraw ()
            return True
        if self.__moving_top:
            self.__moving_top = False
            delta = self.__data.scale_data (self.__moving_top_cur - self.__moving_top_start)
            (left, right) = self.__data.get_range ()
            self.__data.set_range (left+delta, right+delta)
            self.__force_full_redraw ()
        return False
    def motion_notify (self, widget, event):
        (x, y, width, height) = self.__data.get_selection_rectangle ()
        if self.__moving_left:
            if event.x <= 0:
                self.__moving_left_cur = 0
            elif event.x >= x+width:
                self.__moving_left_cur = x+width
            else:
                self.__moving_left_cur = event.x
            self.queue_draw ()
            return True
        if self.__moving_right:
            if event.x >= self.__width:
                self.__moving_right = self.__width
            elif event.x < x:
                self.__moving_right_cur = x
            else:
                self.__moving_right_cur = event.x
            self.queue_draw ()
            return True
        if self.__moving_both:
            cur_e = self.__width - (x + width - self.__moving_both_start)
            cur_s = (self.__moving_both_start - x)
            if event.x < cur_s:
                self.__moving_both_cur = cur_s
            elif event.x > cur_e:
                self.__moving_both_cur = cur_e
            else:
                self.__moving_both_cur = event.x
            self.queue_draw ()
            return True
        if self.__moving_top:
            self.__moving_top_cur = event.x
            self.queue_draw ()
        return False
    def size_allocate (self, widget, allocation):
        self.__width = allocation.width
        self.__height = allocation.height
        self.__data.layout (allocation.width, allocation.height)
        self.__force_full_redraw ()
    def __force_full_redraw (self):
        self.__buffer_surface = cairo.ImageSurface(cairo.FORMAT_ARGB32,
                                                   self.__data.get_width (),
                                                   self.__data.get_height ())
        ctx = cairo.Context(self.__buffer_surface)
        self.__data.draw (ctx)
        self.queue_draw ()
    def expose (self, widget, event):
        ctx = widget.window.cairo_create()
        ctx.rectangle(event.area.x, event.area.y,
                      event.area.width, event.area.height)
        ctx.clip()
        ctx.set_source_surface (self.__buffer_surface)
        ctx.paint ()
        (x, y, width, height) = self.__data.get_selection_rectangle ()
        if self.__moving_left:
            ctx.move_to (max (self.__moving_left_cur, 2), y)
            ctx.rel_line_to (0, height)
            ctx.close_path ()
            ctx.set_line_width (1)
            ctx.set_source_rgb (0,0,0)
            ctx.stroke ()
        if self.__moving_right:
            ctx.move_to (min (self.__moving_right_cur, self.__width-2), y)
            ctx.rel_line_to (0, height)
            ctx.close_path ()
            ctx.set_line_width (1)
            ctx.set_source_rgb (0,0,0)
            ctx.stroke ()
        if self.__moving_both:
            delta_x = self.__moving_both_cur - self.__moving_both_start
            left_x = x + delta_x
            ctx.move_to (x+delta_x, y)
            ctx.rel_line_to (0, height)
            ctx.close_path ()
            ctx.move_to (x+width+delta_x, y)
            ctx.rel_line_to (0, height)
            ctx.close_path ()
            ctx.set_source_rgb (0,0,0)
            ctx.set_line_width (1)
            ctx.stroke ()
        if self.__moving_top:
            (d_x, d_y, d_width, d_height) = self.__data.get_data_rectangle ()
            delta_x = self.__moving_top_cur-self.__moving_top_start
            ctx.move_to (d_x+d_width/2,d_y+d_height/2)
            ctx.rel_line_to (delta_x, 0)
            ctx.close_path ()
            ctx.set_line_width (1)
            ctx.set_source_rgb (0,0,0)
            ctx.stroke ()
        return False

class MainWindow:
    def __init__ (self):
        return
    def run (self, graphic):
        window = gtk.Window()
        window.set_default_size (200, 200)
        vbox = gtk.VBox ()
        window.add (vbox)
        render = GtkGraphicRenderer(graphic)
        self.__render = render
        vbox.pack_end (render, True, True, 0)
        hbox = gtk.HBox ()
        vbox.pack_start (hbox, False, False, 0)
        smaller_zoom = gtk.Button ("Smaller Zoom")
        smaller_zoom.connect ("clicked", self.__set_smaller_cb)
        hbox.pack_start (smaller_zoom)
        bigger_zoom = gtk.Button ("Bigger Zoom")
        bigger_zoom.connect ("clicked", self.__set_bigger_cb)
        hbox.pack_start (bigger_zoom)
        biggest_zoom = gtk.Button ("Biggest Zoom")
        biggest_zoom.connect ("clicked", self.__set_biggest_cb)
        hbox.pack_start (biggest_zoom)
        output_png = gtk.Button ("Output Png")
        output_png.connect ("clicked", self.__output_png_cb)
        hbox.pack_start (output_png)
        window.connect('destroy', gtk.main_quit)
        window.show_all()
        #gtk.bindings_activate (gtk.main_quit, 'q', 0)
        gtk.main()
    def __set_smaller_cb (self, widget):
        self.__render.set_smaller_zoom ()
    def __set_bigger_cb (self, widget):
        self.__render.set_bigger_zoom ()
    def __set_biggest_cb (self, widget):
        self.__render.set_biggest_zoom ()
    def __output_png_cb (self, widget):
        # XXX
        return




def lines_get_range_names (lines):
    names = {}
    for line in lines:
        (first, last) = line.get_bounds ()
        for range in line.get_range (first, last):
            names[range.name] = 1
    return names.keys ()
def lines_sort (lines):
    for line in lines:
        line.sort ()
def lines_get_bounds (lines):
    (lower_bound, upper_bound) = lines[0].get_bounds ()
    for line in lines:
        (first, last) = line.get_bounds ()
        if last > upper_bound:
            upper_bound = last
        if first < lower_bound:
            lower_bound = first
    return (lower_bound, upper_bound)


def main():
    lines = []
    colors = Colors ()
    fh = open(sys.argv[1])
    m1 = re.compile ('range ([^ ]+) ([^ ]+) ([0-9]+) ([0-9]+)')
    m2 = re.compile ('event ([^ ]+) ([^ ]+) ([0-9]+)')
    m3 = re.compile ('color ([^ ]+) #([a-fA-F0-9]{2,2})([a-fA-F0-9]{2,2})([a-fA-F0-9]{2,2})')
    for line in fh.readlines():
        m = m1.match (line)
        if m:
            data_range = DataRange ()
            data_range.name = m.group (2)
            data_range.start = int (m.group (3))
            data_range.end = int (m.group (4))
            line_name = m.group (1)
            found = False
            for line in lines:
                if line.name == line_name:
                    line.add_range (data_range)
                    found = True
                    break
            if not found:
                line = Line (line_name)
                lines.append (line)
                line.add_range (data_range)
            continue
        m = m2.match (line)
        if m:
            event = Event ()
            event.name = m.group (2)
            event.at = int (m.group (3))
            line = lines.lookup (m.group (1))
            line.add_event (event)
            continue
        
        m = m3.match (line)
        if m:
            r = int (m.group (2), 16)
            g = int (m.group (3), 16)
            b = int (m.group (4), 16)
            color = Color (r/255, g/255, b/255)
            colors.add (m.group (1), color)
            continue

    (lower_bound, upper_bound) = lines_get_bounds (lines)
    graphic = GraphicRenderer (lower_bound, upper_bound)
    top_legend = TopLegendRenderer ()
    range_names = lines_get_range_names (lines)
    range_colors = []
    for range_name in range_names:
        range_colors.append (colors.lookup (range_name))
    top_legend.set_legends (range_names,
                            range_colors)
    graphic.set_top_legend (top_legend)
    data = DataRenderer ()
    data.set_data (lines, colors)
    graphic.set_data (data)

    # default range
    range_mid = (upper_bound - lower_bound) /2
    range_width = (upper_bound - lower_bound) /10
    range_lo = range_mid - range_width / 2
    range_hi = range_mid + range_width / 2
    graphic.set_range (range_lo, range_hi)

    main_window = MainWindow ()
    main_window.run (graphic)


main ()
