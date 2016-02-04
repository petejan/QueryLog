import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;

import org.jfree.ui.RefineryUtilities;
import name.pachler.nio.file.*;
import static name.pachler.nio.file.StandardWatchEventKind.*;


public class watchPlot 
{
    private final WatchService watcher;
    private final Map<WatchKey, Path> keys;
    private boolean trace = false;
    
    plot plotPane;
    
    String fileReg;
    String currentFile = "";
    InputStreamReader converter;
    BufferedReader in;

    @SuppressWarnings("unchecked")
    static <T> WatchEvent<T> cast(WatchEvent<?> event)
    {
        return (WatchEvent<T>) event;
    }

    /**
     * Register the given directory with the WatchService
     */
    private void register(Path dir) throws IOException
    {
        WatchKey key = dir.register(watcher, ENTRY_CREATE, ENTRY_DELETE, ENTRY_MODIFY);
        if (trace)
        {
            Path prev = keys.get(key);
            if (prev == null)
            {
                System.out.format("register: %s\n", dir);
            } else
            {
                if (!dir.equals(prev))
                {
                    System.out.format("update: %s -> %s\n", prev, dir);
                }
            }
        }
        keys.put(key, dir);
    }

    /**
     * Creates a WatchService and registers the given directory
     */
    watchPlot(Path dir, String newfileReg, String title, String yaxis, int pos, String reg) throws IOException
    {
        this.watcher = FileSystems.getDefault().newWatchService();
        this.keys = new HashMap<WatchKey, Path>();

        register(dir);
        fileReg = newfileReg;

		plotPane = new plot(title, yaxis, pos, reg);
		plotPane.setContentPane(plotPane.chartPanel);
		plotPane.pack();
		RefineryUtilities.centerFrameOnScreen(plotPane);
		plotPane.setVisible(true);
	        
        
        // enable trace after initial registration
        this.trace = true;
    }

    /**
     * Process all events for keys queued to the watcher
     */
    void processEvents()
    {
    	String curLine;
    	
    	System.out.println("processingEvents");
    	
        for (;;)
        {
            // wait for key to be signalled
            WatchKey key;
            try
            {
                key = watcher.take();
            } 
            catch (InterruptedException x)
            {
                return;
            }

            Path dir = keys.get(key);
            if (dir == null)
            {
                System.err.println("WatchKey not recognized!!");
                continue;
            }

            for (WatchEvent<?> event : key.pollEvents())
            {
                WatchEvent.Kind kind = event.kind();

                // TBD - provide example of how OVERFLOW event is handled
                if (kind == OVERFLOW)
                {
                    continue;
                }

                // Context for directory entry event is the file name of entry
                WatchEvent<Path> ev = cast(event);
                Path name = ev.context();
                Path child = dir.resolve(name);

                // print out event
                System.out.format("%s: %s\n", event.kind().name(), child);
                
                if (child.toString().matches(fileReg))
                {
                	System.out.println("File Match");

                	if ((event.kind() == ENTRY_CREATE) || (in == null))
                	{
                		System.out.println("new File");
                    	try 
                    	{
							converter = new InputStreamReader(new FileInputStream(child.toString()));
	                		in = new BufferedReader(converter);
						} 
                    	catch (FileNotFoundException e) 
                    	{
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
                	}

                	curLine = "";
                	while (curLine != null)
        			{
        				try 
        				{
							curLine = in.readLine();
							// System.out.println("Line " + curLine);
	        				plotPane.read(curLine);
						} 
        				catch (IOException e) 
        				{
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
        			}
                }
            }

            // reset key and remove from set if directory no longer accessible
            boolean valid = key.reset();
            if (!valid)
            {
                keys.remove(key);

                // all directories are inaccessible
                if (keys.isEmpty())
                {
                    break;
                }
            }
        }
    }


	/**
	 * @param args
	 */
	public static void main(String[] args) 
	{
		System.out.println("argc " + args.length);

		String reg = "\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}.*";
		int pos = 5;
		String title = "";
		String yaxis = "value";
		String dir = ".";
		String fileReg = ".*log";

		if (args.length > 0)
		{
			title = args[0];
		}
		else
		{
			System.out.println("Usage: watchPlot <title> <yaxis> <value position> <rexep> <directory> <file filter>");

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
			reg = args[3];
		}
		if(args.length > 4)
		{
			dir = args[4];
		}
		if(args.length > 5)
		{
			fileReg = args[5];
		}
		
		System.out.println("Title          : " + title);
		System.out.println("yAxis          : " + yaxis);
		System.out.println("value position : " + pos);
		System.out.println("reg exp        : " + reg);
		System.out.println("dir            : " + dir);
		System.out.println("file Reg exp   : " + fileReg);

		Path pat = Paths.get(dir);
		try
		{
			new watchPlot(pat, fileReg, title, yaxis, pos, reg).processEvents();
		}
		catch (IOException ex)
		{
			ex.printStackTrace();
		}
 	}

}
