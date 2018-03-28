#include "AppList.hpp"
#include "AppCard.hpp"

AppList::AppList(Get* get, Sidebar* sidebar)
{
	this->x = 400;
	
	// the offset of how far along scroll'd we are
	this->y = 0;
	
	// the main get instance that contains repo info and stuff
	this->get = get;
	
	// the sidebar, which will store the currently selected category info
	this->sidebar = sidebar;
	
	// update current app listing
	update();
}

bool AppList::process(SDL_Event* event)
{
	// only process any events for AppList if there's no subscreen
	if (this->subscreen == NULL)
	{
		// process some joycon input events
		if (event->type == SDL_KEYDOWN)
		{
//			if (event->key.keysym.sym == SDLK_UP)
		}
			
		if (event->type == SDL_MOUSEBUTTONDOWN)
		{
			// make sure that the mouse down's X coordinate is over the app list (not sidebar)
			if (event->motion.x < this->x)
				return false;
			
			// saw mouse down so set it in our element object
			this->dragging = true;
			this->lastMouseY = event->motion.y;
		}
		// drag event for scrolling up or down
		else if (event->type == SDL_MOUSEMOTION)
		{
			if (this->dragging)
			{
				int distance = event->motion.y - this->lastMouseY;
				this->y += distance;
				this->lastMouseY = event->motion.y;
				
				// use the last distance as the rubber band value
				this->elasticCounter = distance;
			}
		}
		else if (event->type == SDL_MOUSEBUTTONUP)
		{
			// mouse up, no more mouse down (TODO: fire selected event here)
			this->dragging = false;
			
			// if the scroll offset is less than the total number of apps
			// (put on the mouse up to make it "snap" when going out of bounds)
			// TODO: account for max number of apps too (prevent scrolling down forever)
			if (this->y > 0)
				this->y = 0;
		}
		
		// if mouse is up, and there's some elastic counter left, burn out remaining elastic value
		if (!this->dragging && this->elasticCounter != 0)
		{
			this->y += this->elasticCounter;
			
			int positivity = this->elasticCounter/abs(this->elasticCounter);
			this->elasticCounter += 10 * (-1 * positivity);
			
			// when the oval and the elastic counter don't match in positivity, reset it to 0
			if (this->elasticCounter != 0 && this->elasticCounter/abs(this->elasticCounter) != positivity)
				this->elasticCounter = 0;
			
			// TODO: same problem as above todo, also extract into method?
			if (this->y > 0)
				this->y = 0;
		}
	}
	
	super::process(event);
	
	return true;
}

void AppList::render(Element* parent)
{
	// draw a white background, 870 wide
	SDL_Rect dimens = { 0, 0, 920, 720 };
	dimens.x = this->x - 35;
	
	SDL_FillRect(parent->window_surface, &dimens, SDL_MapRGBA(parent->window_surface->format, 0xFF, 0xFF, 0xFF, 0xFF));
	this->window_surface = parent->window_surface;
	
	super::render(this);
}

void AppList::update()
{
	// remove any old elements
	this->wipeElements();
	
	// total apps we're interested in so far
	int count = 0;
	
	// the current category value from the sidebar
	std::string curCategoryValue = this->sidebar->currentCatValue();
	
	for (int x=0; x<get->packages.size(); x++)
	{
		// if we're on all categories, or this package matches the current category
		if (curCategoryValue == "*" || curCategoryValue == get->packages[x]->category)
		{
			AppCard* card = new AppCard(get->packages[x]);
			
			this->elements.push_back(card);
			
			// we drew an app, so increase the displayed app counter
			count ++;
		}
	}
	
	// position the filtered app card list
	for (int x=0; x<this->elements.size(); x++)
	{
		// every element after the first should be an app card (we just added them)
		AppCard* card = (AppCard*) elements[x];
		
		// position at proper x, y coordinates
		card->position(10 + (x%3)*265, 130 + 210*(x/3));
		card->update();
	}
	
	// the title of this category (from the sidebar)
	SDL_Color black = { 0, 0, 0, 0xff };
	TextElement* category = new TextElement(this->sidebar->currentCatName().c_str(), 28, &black);
	category->position(20, 90);
	this->elements.push_back(category);
}
