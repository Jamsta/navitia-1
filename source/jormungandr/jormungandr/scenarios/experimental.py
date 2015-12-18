# Copyright (c) 2001-2015, Canal TP and/or its affiliates. All rights reserved.
#
# This file is part of Navitia,
#     the software to build cool stuff with public transport.
#
# Hope you'll enjoy and contribute to this project,
#     powered by Canal TP (www.canaltp.fr).
# Help us simplify mobility and open public transport:
#     a non ending quest to the responsive locomotion way of traveling!
#
# LICENCE: This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Stay tuned using
# twitter @navitia
# IRC #navitia on freenode
# https://groups.google.com/d/forum/navitia
# www.navitia.io

import logging
from flask.ext.restful import abort
from jormungandr.scenarios import new_default
from navitiacommon import type_pb2, response_pb2, request_pb2
from copy import deepcopy
import uuid
from jormungandr.scenarios.utils import fill_uris
from jormungandr.planner import JourneyParameters
from jormungandr.scenarios import journey_filter

from jormungandr.scenarios.new_default import sort_journeys, type_journeys, tag_journeys, culling_journeys

def create_crowfly(_from, to, begin, end, mode='walking'):
    section = response_pb2.Section()
    section.type = response_pb2.CROW_FLY
    section.origin.CopyFrom(_from)
    section.destination.CopyFrom(to)
    section.duration = end-begin;
    section.begin_date_time = begin
    section.end_date_time = end
    section.street_network.mode = response_pb2.Walking
    section.id = str(uuid.uuid4())
    return section

class SectionSorter(object):
    def __call__(self, a, b):
        if a.begin_date_time != b.begin_date_time:
            return -1 if a.begin_date_time < b.begin_date_time else 1
        else:
            return -1 if a.end_date_time < b.end_date_time else 1

class Scenario(new_default.Scenario):

    def __init__(self):
        super(Scenario, self).__init__()



    def journeys(self, request, instance):
        logger = logging.getLogger(__name__)
        logger.warn('using experimental scenario!!!')
        origins = instance.georef.get_stop_points(request['origin'], 'walking', 1800)
        logging.debug('origins: %s', origins)
        destinations = instance.georef.get_stop_points(request['destination'], 'walking', 1800)
        logging.debug('destinations: %s', destinations)

        journey_parameters = JourneyParameters()
        response = instance.planner.journeys(origins, destinations, request['datetime'], request['clockwise'], journey_parameters)
        if not response.journeys:
            return response

        requested_origin = instance.georef.place(request['origin'])
        requested_destination = instance.georef.place(request['destination'])

        for journey in response.journeys:
            departure = journey.sections[0].origin
            arrival = journey.sections[-1].destination

            journey.duration = journey.duration + origins[departure.uri] + destinations[arrival.uri]
            journey.departure_date_time = journey.departure_date_time - origins[departure.uri]
            journey.arrival_date_time = journey.arrival_date_time + destinations[arrival.uri]

            #it's not possible to insert in a protobuf list, so we add the sections at the end, then we sort them
            journey.sections.extend([
                create_crowfly(arrival, requested_destination, journey.sections[-1].end_date_time,
                               journey.arrival_date_time)])
            journey.sections.extend([
                create_crowfly(requested_origin, departure, journey.departure_date_time,
                               journey.sections[0].begin_date_time)])
            journey.sections.sort(SectionSorter())


        journey_filter.filter_journeys([response], instance, request=request, original_request=request)
        sort_journeys(response, instance.journey_order, request['clockwise'])
        tag_journeys(response)
        type_journeys(response, request)
        culling_journeys(response, request)
        fill_uris(response)
        return response



    def nm_journeys(self, request, instance):
        return self.__on_journeys(type_pb2.NMPLANNER, request, instance)

    def isochrone(self, request, instance):
        return self.__on_journeys(type_pb2.ISOCHRONE, request, instance)
