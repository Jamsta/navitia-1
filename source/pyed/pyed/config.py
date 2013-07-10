"""
Class to read the Configuration 
"""
#coding=utf-8
import logging
from ConfigParser import ConfigParser, NoOptionError, NoSectionError

class ConfigException(Exception):
    """ Exception raised when reading the configuration """
    pass

class Config:

    """
    Reads a pyed configuration file

    The file has to contain a instance section with the params name and the 
    path of the directory where there is the following file tree :
    source/ (the directory where to put the data you want to be computed)
    backup/ (you'll find directories with backup data of the data computed)
    error/ (where the error logs go)
    data/ (computed datas)
    gtfs2ed the gtfs2ed program
    osm2ed the osm2ed program
    ed2nav the ed2nav program
    .
    Another section database, with params, name, user, host and password.
    Another one logs_files, with the params : osm2ed, gtfs2ed, ed2nav, pyed.

    Example file : 
    [instance]
    name = instance_name
    source_directory = /srv/ed/instance_name

    [database]
    host = localhost
    name = database_name
    user = user_name
    password = password

    [logs] = 
    osm2ed = /var/log/ed/osm2ed_instance_name
    gtfs2ed = /var/log/ed/gtfs2ed_instance_name
    ed2nav = /var/log/ed/ed2nav_instance_name
    pyed = /var/log/ed/pyed_instance_name
    """
    def __init__(self, filename):
        """ Init the configuration,
            if the file need new fields it can be just added to dico_config
        """
        self.logger = logging.getLogger('root')
        self.logger.info("Initalizing config")
        self.filename = filename
        self.dico_config = {"instance" : {
                                "name" : None,
                                "source_directory" : None
                            }, 
                            "database" : {
                                "name" : None,
                                "user" : None,
                                "host" : None,
                                "password" : None
                                    }, 
                            "logs_files" : {
                                "osm2ed" : None,
                                "gtfs2ed" : None,
                                "ed2nav" : None,
                                "pyed" : None
                                    }
                            }
        self.is_valid_ = False
        self.logger.info("Reading config")
        self.read()
   

    def read(self):
        """ Read and parse the configuration, according to dico_config """
        conf = ConfigParser()
        conf.read(self.filename)
        try:
            for section, options in self.dico_config.iteritems():
                for option_name in options.keys():
                    self.dico_config[section][option_name] = conf.get(section,
                                                                option_name)
        except NoOptionError, exception : 
            self.logger.error(exception.message)
            self.is_valid_ = False
        except NoSectionError, exception:
            self.logger.error(exception.message)
            self.is_valid_ = False
        finally:
            self.is_valid_ = True


    def get(self, section, param_name):
        """ Retrieve the fields idenfied by it section and param_name.
            raise an error if it doesn't exists
        """
        if not section in self.dico_config:
            self.logger.error("Section : " + section + " isn't in the conf")
            raise ConfigException("Section : " + section + " isn't in the conf")
        elif not param_name in self.dico_config[section]:
            self.logger.error("Param : " + section + " isn't in the conf")
            raise ConfigException("Param : " + section + " isn't in the conf")
        return self.dico_config[section][param_name]

    def is_valid(self):
        """ Retrieves if the config is valid or nor """
        return self.is_valid_