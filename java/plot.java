/* ===========================================================
 * JFreeChart : a free chart library for the Java(tm) platform
 * ===========================================================
 *
 * (C) Copyright 2000-2009, by Object Refinery Limited and Contributors.
 *
 * Project Info:  http://www.jfree.org/jfreechart/index.html
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * [Java is a trademark or registered trademark of Sun Microsystems, Inc.
 * in the United States and other countries.]
 *
 * -------------------------
 * TimeSeriesChartDemo1.java
 * -------------------------
 * (C) Copyright 2003-2009, by Object Refinery Limited and Contributors.
 *
 * Original Author:  David Gilbert (for Object Refinery Limited);
 * Contributor(s):   ;
 *
 * Changes
 * -------
 * 09-Mar-2005 : Version 1, copied from the demo collection that ships with
 *               the JFreeChart Developer Guide (DG);
 *
 */

import java.awt.Color;
import java.text.SimpleDateFormat;

import javax.swing.JPanel;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

import java.util.StringTokenizer;

import java.util.Date;
import java.util.regex.Pattern;
import java.text.ParseException;

import java.lang.NumberFormatException;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.time.Millisecond;
import org.jfree.data.time.Month;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYDataset;
import org.jfree.ui.ApplicationFrame;
import org.jfree.ui.RectangleInsets;
import org.jfree.ui.RefineryUtilities;

/**
 * An example of a time series chart. For the most part, default settings are
 * used, except that the renderer is modified to show filled shapes (as well as
 * lines) at each data point.
 */
public class plot extends ApplicationFrame
{
	TimeSeries s1;
	ChartPanel chartPanel;
	int pos;
	String rex;

	/**
	 * A demonstration application showing how to create a simple time series
	 * chart. This example uses monthly data.
	 * 
	 * @param title
	 *            the frame title.
	 */
	public plot(String title, String yaxis, int newpos, String newreg)
	{
		super(title);
		
		pos = newpos;
		rex = newreg;

		chartPanel = (ChartPanel) createDemoPanel(title, yaxis);
		chartPanel.setPreferredSize(new java.awt.Dimension(600, 400));

	}

	/**
	 * Creates a chart.
	 * 
	 * @param dataset
	 *            a dataset.
	 * 
	 * @return A chart.
	 */
	private JFreeChart createChart(XYDataset dataset, String title, String yaxis)
	{
		JFreeChart chart = ChartFactory.createTimeSeriesChart(
				title, // title
				"Date", // x-axis label
				yaxis, // y-axis label
				dataset, // data
				false, // create legend?
				true, // generate tooltips?
				false // generate URLs?
				);

		chart.setBackgroundPaint(Color.white);

		XYPlot plot = (XYPlot) chart.getPlot();
		plot.setBackgroundPaint(Color.lightGray);
		plot.setDomainGridlinePaint(Color.white);
		plot.setRangeGridlinePaint(Color.white);
		plot.setAxisOffset(new RectangleInsets(0.0, 0.0, 0.0, 0.0));
		plot.setDomainCrosshairVisible(true);
		plot.setRangeCrosshairVisible(true);

		XYItemRenderer r = plot.getRenderer();
		if (r instanceof XYLineAndShapeRenderer)
		{
			XYLineAndShapeRenderer renderer = (XYLineAndShapeRenderer) r;
			renderer.setBaseShapesVisible(true);
			renderer.setBaseShapesFilled(true);
			renderer.setDrawSeriesLineAsPath(true);
		}

		DateAxis axis = (DateAxis) plot.getDomainAxis();
		axis.setDateFormatOverride(new SimpleDateFormat("HH:mm"));

		return chart;

	}

	/**
	 * Creates a dataset, consisting of two series of monthly data.
	 * 
	 * @return The dataset.
	 */
	private XYDataset createDataset(String yaxis)
	{
		s1 = new TimeSeries(yaxis);
		
		s1.setMaximumItemAge(24 * 60 * 60 * 1000); // 24 hour of data max

		TimeSeriesCollection dataset = new TimeSeriesCollection();
		dataset.addSeries(s1);

		return dataset;

	}

	/**
	 * Creates a panel for the demo (used by SuperDemo.java).
	 * 
	 * @return A panel.
	 */
	public JPanel createDemoPanel(String title, String yaxis)
	{
		JFreeChart chart = createChart(createDataset(yaxis), title, yaxis);
		ChartPanel panel = new ChartPanel(chart);
		panel.setFillZoomRectangle(true);
		panel.setMouseWheelEnabled(true);

		return panel;
	}
	
	public void read(String curLine)
	{
		Pattern myPattern = Pattern.compile(rex);
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
		String[] split;
		Date ts;
		double value;
		
		if (curLine != null)
		{
			System.out.println("Match " + curLine.matches(rex) + " line: " + curLine.substring(0, Math.min(70, curLine.length())));
			
			if (myPattern.matcher(curLine).find())
			{
				split = curLine.split(",");
				for(int i=0;i<split.length;i++)
				{
					//System.out.println(i + " " + split[i]);
				}

				if (split.length >= pos)
				{
					try
					{
						ts = sdf.parse(split[0]);
						value = Double.parseDouble(split[pos]);

						System.out.println(sdf.format(ts) + " value " + value);

						s1.add(new Millisecond(ts), value);
					}
					catch (NumberFormatException e1)
					{
						e1.printStackTrace();
					}
					catch (ParseException e2)
					{
						e2.printStackTrace();
					}
				}
			}
		}
		
	}

	public void read(BufferedReader in)
	{
		try
		{
			String curLine = "";

			while (curLine != null)
			{
				curLine = in.readLine();
				read(curLine);
			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
	}
	
	/**
	 * Starting point for the demonstration application.
	 * 
	 * @param args
	 *            ignored.
	 */
	public static void main(String[] args)
	{
		System.out.println("argc " + args.length);

		String rex = "\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}.*";
		int pos = 5;
		String title = "";
		String yaxis = "value";

		if (args.length > 0)
		{
			title = args[0];
		}
		else
		{
			System.out.println("Usage: plot <title> <yaxis> <value position> <rexep>");

			System.exit(-1);
		}
		if(args.length > 1)
		{
			yaxis = args[1];
		}
		if(args.length > 2)
		{
			pos = Integer.parseInt(args[2]);
		}
		if(args.length > 3)
		{
			rex = args[3];
			System.out.println("rex " + rex);
		}

		plot demo = new plot(title, yaxis, pos, rex);
		demo.setContentPane(demo.chartPanel);
		demo.pack();
		RefineryUtilities.centerFrameOnScreen(demo);
		demo.setVisible(true);

		InputStreamReader converter = new InputStreamReader(System.in);
		BufferedReader in = new BufferedReader(converter);

		demo.read(in);
	}
}
