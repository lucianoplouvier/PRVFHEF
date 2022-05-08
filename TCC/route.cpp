#include "route.h"

RouteCreator::RouteCreator() {

}

RouteCreator::~RouteCreator() {

}

Route RouteCreator::createRoute() {
	Route r;
	r.id = m_lastId;
	m_lastId = m_lastId + 1;
	return r;
}