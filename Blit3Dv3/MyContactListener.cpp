#include "MyContactListener.h"
 
MyContactListener::MyContactListener() : contacts() 
{
}
 
MyContactListener::~MyContactListener() 
{
}
 
void MyContactListener::BeginContact(b2Contact* contact) {
    // We need to copy out the data because the b2Contact passed in
    // is reused.
    MyContact myContact = { contact->GetFixtureA(), contact->GetFixtureB() };
	myContact.maxImpulseAB = 0.f;
    contacts.push_back(myContact);
}
 
void MyContactListener::EndContact(b2Contact* contact) {
    MyContact myContact = { contact->GetFixtureA(), contact->GetFixtureB() };
    
    pos = std::find(contacts.begin(), contacts.end(), myContact);
    if (pos != contacts.end()) 
	{
        contacts.erase(pos);
    }
}
 
void MyContactListener::PreSolve(b2Contact* contact, 
  const b2Manifold* oldManifold) 
{
	//nothing here for now
}
 
void MyContactListener::PostSolve(b2Contact* contact, 
  const b2ContactImpulse* impulse) 
{
	MyContact myContact = { contact->GetFixtureA(), contact->GetFixtureB() };
    
    pos = std::find(contacts.begin(), contacts.end(), myContact);
    if (pos != contacts.end()) 
	{
		//...world manifold is helpful for getting locations
		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);

		// find the strength of the impulse..
		int32 count = contact->GetManifold()->pointCount; //find all the points of contact
		float maxImpulse = 0.0f;
		for (int32 i = 0; i < count; ++i) 
		{
			//figure out which contact point collided with the maximum force
			maxImpulse = b2Max(maxImpulse, impulse->normalImpulses[i]);
		}

		pos->maxImpulseAB = maxImpulse;

		//normal
		pos->normal = worldManifold.normal;

		//contact point: just use the first one for now
		pos->contactPoint = worldManifold.points[0];
	}
} 
