package com.android.myfirstapp;

import java.util.Stack;

import org.xml.sax.Attributes;
import org.xml.sax.helpers.DefaultHandler;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.Log;

public class SAXEventHandler extends DefaultHandler {

	/** List des markers */
	private ListeMarkers listeDesMarkers;

	/** Pile utilisée pour le parsing */
    private Stack<Object> stack;

    private Context context;
    
    private Drawable defaultMarker;
    
    /** Constructeur de la classe */
    public SAXEventHandler(Context context, Drawable defaultMarker) {
    	super();
    	this.stack = new Stack<Object>();
    	this.context = context;
    	this.defaultMarker = defaultMarker;
    }

	/** @return la liste listeDesMarkers */
	public ListeMarkers getListeDesMarkers() {
		return listeDesMarkers;
	}

	/**
	 * 
	 */
	public void startElement(String uri, String localName,
							String qName, Attributes atts) {
		if(localName.equals("markers")) {
			// Mise dans la pile d'un élément <markers>
			stack.push(new ListeMarkers(context, defaultMarker));
		}
		else {
			if(localName.equals("marker")) {
				// Mise dans la pile d'un élément <marker> (sans S final cette fois)
				Marker nMarker = new Marker(atts.getValue(uri, "nom"),
											atts.getValue(uri, "adresse"),
											atts.getValue(uri, "lat"),
											atts.getValue(uri, "lng"));
				stack.push(nMarker);
			} else {
				// TODO GERER LE CAS D'ERREUR
				Log.d("SAX ECO", "ELEMENT INATTENDU");
			}
		}
	}
	
	/**
	 * 
	 */
	public void endElement(String uri, String localName,
							String qName) {
		// pop stack and add to 'parent' element, which is next on the stack
		// important to pop stack first, then peek at top element!
		Object tmp = stack.pop();
		
		if( localName.equals("markers")) {
			this.listeDesMarkers = (ListeMarkers)tmp;
		}
		else {
			if(localName.equals("marker") ) {
			    ((ListeMarkers)stack.peek()).addMarker((Marker)tmp);
			}
			else {
				// TODO GERER LE CAS D'ERREUR
				Log.d("SAX ECO", "ELEMENT INATTENDU");
			    stack.push(tmp);
			}
		}
	}
}