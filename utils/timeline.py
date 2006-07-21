#!/usr/bin/env python

import re
import sys
import cairo
import gtk

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
class DataRange:
    def __init__ (self, start = 0, end = 0, value = ''):
        self.start = start
        self.end = end
        self.value = value
class EventString:
    def __init__ (self, at = 0, value = ''):
        self.at = at
        self.value = value
class EventFloat:
    def __init__ (self, at = 0, value = 0.0):
        self.at = at
        self.value = value
class EventInt:
    def __init__ (self, at = 0, value = 0.0):
        self.at = at
        self.value = value
class TimelineDataRange:
    def __init__ (self, name = ''):
        self.name = name
        self.ranges = []
        return
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
    def add_range (self, range):
        self.ranges.append (range)
    def get_ranges (self, start, end):
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
    def sort (self):
        self.ranges.sort ()
    def get_bounds (self):
        if len (self.ranges) > 0:
            lo = self.ranges[0].start
            hi = self.ranges[len (self.ranges)-1].end
            return (lo, hi)
        else:
            return (0,0)
class TimelineEvent:
    def __init__ (self, name = ''):
        self.name = name
        self.events = []
    def __search (self, key):
        l = 0
        u = len (self.events)-1
        while l <= u:
            i = int ((l+u)/2)
            if key == self.events[i].at:
                return i
            elif key < self.events[i].at:
                u = i - 1
            else:
                # key > self.events[i].at
                l = i + 1
        return l
    def add_event (self, event):
        self.events.append (event)
    def get_events (self, start, end):
        s = self.__search (start)
        e = self.__search (end)
        return self.events[s:e+1]
    def sort (self):
        self.events.sort ()
    def get_bounds (self):
        if len (self.events) > 0:
            lo = self.events[0].at
            hi = self.events[-1].at
            return (lo,hi)
        else:
            return (0,0)

class Timeline:
    def __init__ (self, name = ''):
        self.ranges = []
        self.event_str = []
        self.event_int = []
        self.name = name
    def get_range (self, name):
        for range in self.ranges:
            if range.name == name:
                return range
        timeline = TimelineDataRange (name)
        self.ranges.append (timeline)
        return timeline
    def get_event_str (self, name):
        for event_str in self.event_str:
            if event_str.name == name:
                return event_str
        timeline = TimelineEvent (name)
        self.event_str.append (timeline)
        return timeline
    def get_event_int (self, name):
        for event_int in self.event_int:
            if event_int.name == name:
                return event_int
        timeline = TimelineEvent (name)
        self.event_int.append (timeline)
        return timeline
    def get_ranges (self):
        return self.ranges
    def get_events_str (self):
        return self.event_str
    def get_events_int (self):
        return self.event_int
    def sort (self):
        for range in self.ranges:
            range.sort ()
        for event in self.event_int:
            event.sort ()
        for event in self.event_str:
            event.sort ()
    def get_bounds (self):
        lo = 0
        hi = 0
        for range in self.ranges:
            (range_lo, range_hi) = range.get_bounds ()
            if range_lo < lo:
                lo = range_lo
            if range_hi > hi:
                hi = range_hi
        for event_str in self.event_str:
            (ev_lo, ev_hi) = event_str.get_bounds ()
            if ev_lo < lo:
                lo = ev_lo
            if ev_hi > hi:
                hi = ev_hi
        for event_int in self.event_int:
            (ev_lo, ev_hi) = event_int.get_bounds ()
            if ev_lo < lo:
                lo = ev_lo
            if ev_hi > hi:
                hi = ev_hi
        return (lo, hi)

class Timelines:
    def __init__ (self):
        self.timelines = []
    def get (self, name):
        for timeline in self.timelines:
            if timeline.name == name:
                return timeline
        timeline = Timeline (name)
        self.timelines.append (timeline)
        return timeline
    def get_all (self):
        return self.timelines
    def sort (self):
        for timeline in self.timelines:
            timeline.sort ()
    def get_bounds (self):
        lo = 0
        hi = 0
        for timeline in self.timelines:
            (t_lo, t_hi) = timeline.get_bounds ()
            if t_lo < lo:
                lo = t_lo
            if t_hi > hi:
                hi = t_hi
        return (lo, hi)

class TimelinesRenderer:
    def __init__ (self):
        self.padding = 10
        return
    def get_height (self):
        return self.height
    def set_timelines (self, timelines, colors):
        self.timelines = timelines
        self.colors = colors
    def set_render_range (self, start, end):
        self.start = start
        self.end = end
    def layout (self, width):
        surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 1,1)
        ctx = cairo.Context(surface)
        max_text_height = ctx.text_extents ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcedefghijklmnopqrstuvwxyz0123456789")[3]

        left_width = 0
        right_width = 0
        left_n_lines = 0
        range_n = 0
        eventint_n = 0
        eventstr_n = 0
        for timeline in self.timelines.get_all ():
            left_n_lines += 1
            t_width = ctx.text_extents (timeline.name)[2]
            left_width = max (left_width, t_width)
            for rang in timeline.get_ranges ():
                t_width = ctx.text_extents (rang.name)[2]
                right_width = max (right_width, t_width)
                range_n += 1
            for events_int in timeline.get_events_int ():
                t_width = ctx.text_extents (events_int.name)[2]
                right_width = max (right_width, t_width)
                eventint_n += 1
            for events_str in timeline.get_events_str ():
                t_width = ctx.text_extents (events_str.name)[2]
                right_width = max (right_width, t_width)
                eventstr_n += 1

        left_height = left_n_lines * max_text_height + (left_n_lines - 1) * self.padding
        right_n_lines = range_n + eventint_n + eventstr_n
        right_height = (right_n_lines - 1) * self.padding + right_n_lines * max_text_height
        right_data_height = (eventint_n + eventstr_n) * (max_text_height + 5) + range_n * 10
        right_data_height += (right_n_lines - 1) * self.padding

        height = max (left_height, right_height)
        height = max (height, right_data_height)

        self.left_width = left_width
        self.right_width = right_width
        self.max_text_height = max_text_height
        self.width = width
        self.height = height
    def draw_line (self, ctx, x, y, width, height):
        ctx.move_to (x, y)
        ctx.rel_line_to (width, height)
        ctx.set_operator (cairo.OPERATOR_SOURCE)
        ctx.set_line_width (1)
        ctx.set_source_rgb (0,0,0)
        ctx.stroke ()
    def draw_events (self, ctx, events, x, y, width, height):
        if (self.grey_background % 2) == 0:
            ctx.rectangle (x, y-self.padding/2,
                           width, height+self.padding)
            ctx.set_source_rgb (0.9,0.9,0.9)
            ctx.fill ()
        last_x_drawn = int (x)
        for event in events.get_events (self.start, self.end):
            real_x = int (x + (event.at - self.start) * width / (self.end - self.start))
            if real_x > last_x_drawn:
                ctx.rectangle (real_x, y, 1, 1)
                ctx.set_source_rgb (1,0,0)
                ctx.stroke ()
                ctx.move_to (real_x, y+self.max_text_height)
                ctx.set_source_rgb (0,0,0)
                ctx.show_text (str (event.value))
        self.grey_background += 1
    def draw_ranges (self, ctx, ranges, x, y, width, height):
        if (self.grey_background % 2) == 0:
            ctx.rectangle (x, y-self.padding/2,
                           width, height+self.padding)
            ctx.set_source_rgb (0.9,0.9,0.9)
            ctx.fill ()
        last_x_drawn = int (x)
        for data_range in ranges.get_ranges (self.start, self.end):
            x_start = int (x + (data_range.start - self.start) * width / (self.end - self.start))
            x_end = int (x + (data_range.end - self.start) * width / (self.end - self.start))
            if x_start > last_x_drawn:
                ctx.rectangle (x_start, y, x_end - x_start, 10)
                ctx.set_source_rgb (0,0,0)
                ctx.stroke_preserve ()
                color = self.colors.lookup (data_range.value)
                ctx.set_source_rgb (color.r, color.g, color.b)
                ctx.fill ()
                last_x_drawn = x_end

        self.grey_background += 1
        
    def draw (self, ctx):
        timeline_top = 0
        top_y = self.padding / 2
        left_x_start = self.padding / 2
        left_x_end = left_x_start + self.left_width
        right_x_start = left_x_end + self.padding
        right_x_end = right_x_start + self.right_width
        data_x_start = right_x_end + self.padding /2
        data_x_end = self.width
        data_width = data_x_end - data_x_start
        cur_y = top_y
        self.draw_line (ctx, 0, 0, self.width, 0)
        self.grey_background = 1
        for timeline in self.timelines.get_all ():
            (y_bearing,t_width,t_height) = ctx.text_extents (timeline.name)[1:4]
            ctx.move_to (left_x_start, cur_y + self.max_text_height - (t_height+y_bearing))
            ctx.show_text (timeline.name);
            for events_int in timeline.get_events_int ():
                (y_bearing, t_width, t_height) = ctx.text_extents (events_int.name)[1:4]
                ctx.move_to (right_x_start, cur_y + self.max_text_height - (t_height+y_bearing))
                ctx.show_text (events_int.name)
                self.draw_events (ctx, events_int, data_x_start, cur_y, data_width, self.max_text_height+5)
                cur_y += self.max_text_height + 5 + self.padding
                self.draw_line (ctx, right_x_start-self.padding/2, cur_y - self.padding / 2,
                                self.right_width + self.padding, 0)

            for events_str in timeline.get_events_str ():
                (y_bearing, t_width, t_height) = ctx.text_extents (events_str.name)[1:4]
                ctx.move_to (right_x_start, cur_y + self.max_text_height - (t_height+y_bearing))
                ctx.show_text (events_str.name)
                self.draw_events (ctx, events_str, data_x_start, cur_y, data_width, self.max_text_height+5)
                cur_y += self.max_text_height + 5 + self.padding
                self.draw_line (ctx, right_x_start-self.padding/2, cur_y - self.padding / 2,
                                self.right_width + self.padding, 0)
            for ranges in timeline.get_ranges ():
                (y_bearing, t_width, t_height) = ctx.text_extents (ranges.name)[1:4]
                ctx.move_to (right_x_start, cur_y + self.max_text_height - (t_height+y_bearing))
                ctx.show_text (ranges.name)
                self.draw_ranges (ctx, ranges, data_x_start, cur_y, data_width, 10)
                cur_y += self.max_text_height + self.padding
                self.draw_line (ctx, right_x_start-self.padding/2, cur_y - self.padding / 2,
                                self.right_width + self.padding, 0)
            self.draw_line (ctx, 0, cur_y - self.padding / 2,
                            self.width, 0)
        bot_y = cur_y - self.padding / 2
        self.draw_line (ctx, left_x_end+self.padding/2, 0,
                        0, bot_y)
        self.draw_line (ctx, right_x_end+self.padding/2, 0,
                        0, bot_y)
        return


class GtkTimelines (gtk.DrawingArea):
    def __init__ (self):
        super (GtkTimelines, self).__init__ ()
        self.connect ("expose_event", self.expose)
        self.connect ('size-allocate', self.size_allocate)
        return
    def set_timelines (self, timelines):
        self.timelines = timelines
    def expose (self, widget, event):
        ctx = widget.window.cairo_create()
        ctx.rectangle(event.area.x, event.area.y,
                      event.area.width, event.area.height)
        #ctx.clip ()
        ctx.clip_preserve ()
        ctx.set_source_rgb (1, 1, 1)
        ctx.set_operator (cairo.OPERATOR_SOURCE)
        ctx.fill ()

        self.timelines.draw (ctx)
        return
    def size_allocate (self, widget, allocation):
        self.width = allocation.width
        self.height = allocation.height
        self.timelines.layout (self.width)
        return



    
def read_data(filename):
    timelines = Timelines ()
    colors = Colors ()
    fh = open(filename)
    m1 = re.compile ('range ([^ ]+) ([^ ]+) ([^ ]+) ([0-9]+) ([0-9]+)')
    m2 = re.compile ('event-str ([^ ]+) ([^ ]+) ([^ ]+) ([0-9]+)')
    m3 = re.compile ('event-int ([^ ]+) ([^ ]+) ([0-9]+) ([0-9]+)')
    m4 = re.compile ('color ([^ ]+) #([a-fA-F0-9]{2,2})([a-fA-F0-9]{2,2})([a-fA-F0-9]{2,2})')
    for line in fh.readlines():
        m = m1.match (line)
        if m:
            line_name = m.group (1)
            timeline = timelines.get (m.group (1))
            rang = timeline.get_range (m.group (2))
            data_range = DataRange ()
            data_range.value = m.group (3)
            data_range.start = int (m.group (4))
            data_range.end = int (m.group (5))
            rang.add_range (data_range)
            continue
        m = m2.match (line)
        if m:
            line_name = m.group (1)
            timeline = timelines.get (m.group (1))
            ev = timeline.get_event_str (m.group (2))
            event = EventString ()
            event.value = m.group (3)
            event.at = int (m.group (4))
            ev.add_event (event)
            continue
        m = m3.match (line)
        if m:
            line_name = m.group (1)
            timeline = timelines.get (m.group (1))
            ev = timeline.get_event_int (m.group (2))
            event = EventInt ()
            event.value = int (m.group (3))
            event.at = int (m.group (4))
            ev.add_event (event)
            continue
        
        m = m4.match (line)
        if m:
            r = int (m.group (2), 16)
            g = int (m.group (3), 16)
            b = int (m.group (4), 16)
            color = Color (r/255, g/255, b/255)
            colors.add (m.group (1), color)
            continue
    timelines.sort ()
    return (colors, timelines)

(colors, timelines) = read_data (sys.argv[1])
(start, end) = timelines.get_bounds ()

window = gtk.Window()
window.set_default_size (200, 200)
timelines_renderer = TimelinesRenderer ()
timelines_renderer.set_timelines (timelines, colors)
timelines_renderer.set_render_range (start, end)
gtk_timeline = GtkTimelines ()
gtk_timeline.set_timelines (timelines_renderer)
window.add (gtk_timeline)
window.show_all ()
gtk.main ()
