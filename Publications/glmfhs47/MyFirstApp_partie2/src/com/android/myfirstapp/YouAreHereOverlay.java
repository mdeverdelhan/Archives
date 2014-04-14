package com.android.myfirstapp;

import android.graphics.drawable.Drawable;

import com.google.android.maps.ItemizedOverlay;
import com.google.android.maps.OverlayItem;

public class YouAreHereOverlay extends ItemizedOverlay<OverlayItem> {

	private OverlayItem youAreHereItem;
	
	public YouAreHereOverlay(Drawable defaultMarker) {
		super(boundCenterBottom(defaultMarker));
	}
	
	public void setYouAreHereItem(OverlayItem youAreHereItem) {
		this.youAreHereItem = youAreHereItem;
		populate();
	}
	
	@Override
	protected OverlayItem createItem(int i) {
		return youAreHereItem;
	}

	@Override
	public int size() {
		return 1;
	}

}
