

/*
 *    AsTeRICS - Assistive Technology Rapid Integration and Construction Set
 * 
 * 
 *        d8888      88888888888       8888888b.  8888888 .d8888b.   .d8888b. 
 *       d88888          888           888   Y88b   888  d88P  Y88b d88P  Y88b
 *      d88P888          888           888    888   888  888    888 Y88b.     
 *     d88P 888 .d8888b  888   .d88b.  888   d88P   888  888         "Y888b.  
 *    d88P  888 88K      888  d8P  Y8b 8888888P"    888  888            "Y88b.
 *   d88P   888 "Y8888b. 888  88888888 888 T88b     888  888    888       "888
 *  d8888888888      X88 888  Y8b.     888  T88b    888  Y88b  d88P Y88b  d88P
 * d88P     888  88888P' 888   "Y8888  888   T88b 8888888 "Y8888P"   "Y8888P" 
 *
 *
 *                    homepage: http://www.asterics.org 
 *
 *       This project has been partly funded by the European Commission, 
 *                      Grant Agreement Number 247730
 *  
 *  
 *         Dual License: MIT or GPL v3.0 with "CLASSPATH" exception
 *         (please refer to the folder LICENSE)
 * 
 */

package eu.asterics.component.actuator.textdisplay;


import java.util.HashMap;
import java.util.Iterator;
import java.util.logging.Logger;

import eu.asterics.mw.data.ConversionUtils;
import eu.asterics.mw.model.runtime.AbstractRuntimeComponentInstance;
import eu.asterics.mw.model.runtime.IRuntimeInputPort;
import eu.asterics.mw.model.runtime.impl.DefaultRuntimeInputPort;
import eu.asterics.mw.model.runtime.IRuntimeOutputPort;
import eu.asterics.mw.model.runtime.IRuntimeEventListenerPort;
import eu.asterics.mw.model.runtime.IRuntimeEventTriggererPort;
import eu.asterics.mw.model.runtime.impl.DefaultRuntimeOutputPort;
import eu.asterics.mw.model.runtime.impl.DefaultRuntimeEventTriggererPort;
import eu.asterics.mw.services.AstericsErrorHandling;
import eu.asterics.mw.services.AREServices;

enum TextPosition {
    Left,Center,Right
};

/**
 * 
 * Implements the Text GUI component
 * 
 * 
 * @author Karol Pecyna [kpecyna@harpo.com.pl]
 *         Date: Dec 19, 2011
 *         Time: 12:31:41 AM
 */
public class TextDisplayInstance extends AbstractRuntimeComponentInstance
{
	// Usage of an output port e.g.: opMyOutPort.sendData(ConversionUtils.intToBytes(10)); 

	// Usage of an event trigger port e.g.: etpMyEtPort.raiseEvent();

	private final String PROP_CAPTION ="caption";
	private final String PROP_DEFAULT="default";
	private final String PROP_TEXT_POSITION="textPosition";
	private final String PROP_TEXT_COLOR="textColor";
	private final String PROP_BACKGROUND_COLOR="backgroundColor";
	private final String ETP_CLICKED = "clicked";
	private final String ELP_CLEAR="clear";
	
	String propDefault = "";
	String propCaption = "Text Display";
	int propTextPosition=1;
	int propTextColor=0;
	int propBackgroundColor=11;
    public boolean propDisplayGUI=true;
	
	final IRuntimeEventTriggererPort etpClicked = new DefaultRuntimeEventTriggererPort();
	
	private  GUI gui = null;
	private boolean guiReady=false;
  
    
   /**
    * The class constructor.
    */
    public TextDisplayInstance()
    {
        // empty constructor
    }

   /**
    * returns an Input Port.
    * @param portID   the name of the port
    * @return         the input port or null if not found
    */
    public IRuntimeInputPort getInputPort(String portID)
    {
		if ("input".equalsIgnoreCase(portID))
		{
			return ipInput;
		}

		return null;
	}

    /**
     * returns an Output Port.
     * @param portID   the name of the port
     * @return         the output port or null if not found
     */
    public IRuntimeOutputPort getOutputPort(String portID)
	{

		return null;
	}

    /**
     * Returns an Event Listener Port.
     * @param eventPortID   the name of the port
     * @return         the event listener port or null if not found
     */
    @Override
    public IRuntimeEventListenerPort getEventListenerPort(String eventPortID)
    {
      if(ELP_CLEAR.equalsIgnoreCase(eventPortID))
      {
        return elpClear;
      }
      else
      {
        return null;
      }
    }
    
    /**
     * Input event port.
     */
    final IRuntimeEventListenerPort elpClear 	= new IRuntimeEventListenerPort()
    {
      @Override 
      public void receiveEvent(String data)
      {
    	  if(guiReady)
    	  {
    		  gui.setText("");
    	  }	  
      }
    };    

    /**
     * returns an Event Triggerer Port.
     * @param eventPortID   the name of the port
     * @return         the EventTriggerer port or null if not found
     */
    public IRuntimeEventTriggererPort getEventTriggererPort(String eventPortID)
    {

       if(ETP_CLICKED.equalsIgnoreCase(eventPortID))
       {
          return etpClicked;
       }
       return null;
    }
		
    /**
     * returns the value of the given property.
     * @param propertyName   the name of the property
     * @return               the property value or null if not found
     */
    public Object getRuntimePropertyValue(String propertyName)
    {
		if (PROP_DEFAULT.equalsIgnoreCase(propertyName))
		{
			return propDefault;
		}
		if (PROP_CAPTION.equalsIgnoreCase(propertyName))
		{
			return propCaption;
		}
		if (PROP_TEXT_POSITION.equalsIgnoreCase(propertyName))
		{
			return propTextPosition;
		}
		if (PROP_TEXT_COLOR.equalsIgnoreCase(propertyName))
		{
			return propTextColor;
		}
		if (PROP_BACKGROUND_COLOR.equalsIgnoreCase(propertyName))
		{
			return propBackgroundColor;
		}
    	if("displayGUI".equalsIgnoreCase(propertyName))
        {
            return propDisplayGUI;
        }

        return null;
    }

    /**
     * sets a new value for the given property.
     * @param propertyName   the name of the property
     * @param newValue       the desired property value or null if not found
     */
    public Object setRuntimePropertyValue(String propertyName, Object newValue)
    {
		if (PROP_DEFAULT.equalsIgnoreCase(propertyName))
		{
			final Object oldValue = propDefault;
			propDefault = (String)newValue;
			return oldValue;
		}
		if (PROP_CAPTION.equalsIgnoreCase(propertyName))
		{
			final Object oldValue = propCaption;
			propCaption = (String)newValue;
			return oldValue;
		}
		if (PROP_TEXT_POSITION.equalsIgnoreCase(propertyName))
		{
			final Object oldValue = propTextPosition;
			propTextPosition = Integer.parseInt((String) newValue);
			if((propTextPosition<0)||(propTextPosition>2))
			{
				propTextPosition=1;
			}
			return oldValue;
		}
		if (PROP_TEXT_COLOR.equalsIgnoreCase(propertyName))
		{
			final Object oldValue = propTextColor;
			propTextColor = Integer.parseInt((String) newValue);
			if((propTextColor<0)||(propTextColor>12))
			{
				propTextColor=0;
			}
			return oldValue;
		}
		if (PROP_BACKGROUND_COLOR.equalsIgnoreCase(propertyName))
		{
			final Object oldValue = propBackgroundColor;
			propBackgroundColor = Integer.parseInt((String) newValue);
			if((propBackgroundColor<0)||(propBackgroundColor>12))
			{
				propBackgroundColor=11;
			}
			return oldValue;
		}
    	if("displayGUI".equalsIgnoreCase(propertyName))
        {
            final Object oldValue = propDisplayGUI;

            if("true".equalsIgnoreCase((String)newValue))
            {
            	propDisplayGUI = true;
            }
            else if("false".equalsIgnoreCase((String)newValue))
            {
            	propDisplayGUI = false;
            }
            return oldValue;
        }    	

        return null;
    }

     /**
      * Input Ports for receiving values.
      */
	private final IRuntimeInputPort ipInput  = new DefaultRuntimeInputPort()
	{
		public void receiveData(byte[] data)
		{
			String text = ConversionUtils.stringFromBytes(data);
			if(guiReady)
			{
				gui.setText(text);
			}
		}

		
	};


     /**
      * called when model is started.
      */
      @Override
      public void start()
      {
			gui = new GUI(this,AREServices.instance.getAvailableSpace(this));
			if (propDisplayGUI) AREServices.instance.displayPanel(gui, this, true);
			guiReady=true;
          super.start();
      }

     /**
      * called when model is paused.
      */
      @Override
      public void pause()
      {
          super.pause();
          
      }

     /**
      * called when model is resumed.
      */
      @Override
      public void resume()
      {
          super.resume();
          
      }

     /**
      * called when model is stopped.
      */
      @Override
      public void stop()
      {
			AREServices.instance.displayPanel(gui, this, false);
			guiReady=false;
          super.stop();
      }
      
      /**
       * Returns the plugin caption.
       * @return   plugin caption
       */
      String getCaption()
      {
        return propCaption;
      }
      
      /**
       * Returns the default text.
       * @return   default text
       */
      String getDefaultText()
      {
        return propDefault;
      }
      
      /**
       * Returns the text color.
       * @return   text color
       */
      int getTextColor()
      {
    	  return propTextColor;
      }
      
      /**
       * Returns the background color.
       * @return   background color
       */
      int getBackgroundColor()
      {
    	  return propBackgroundColor;
      }
      
      /**
       * Returns the position of the text.
       * @return   text position.
       */
      TextPosition getTextPosition()
      {
        
    	TextPosition position=TextPosition.Center;
    	switch(propTextPosition)
        {
        case 0:
        	position=TextPosition.Left;
        	break;
        case 1:
        	position=TextPosition.Center;
        	break;
        case 2:
        	position=TextPosition.Right;
        	break;
        }
    	
    	return position; 
      }
      
      public void syncedValuesReceived (HashMap <String, byte[]> dataRow)
      {
    		
    	  Iterator iterator = dataRow.keySet().iterator();
    	  if(guiReady)
			{
				gui.setText(ConversionUtils.stringFromBytes(dataRow.get("input")));
			}
      }
}