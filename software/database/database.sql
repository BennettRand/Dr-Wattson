--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: device; Type: TABLE; Schema: public; Owner: root; Tablespace: 
--

CREATE TABLE device (
    id integer NOT NULL,
    addr inet NOT NULL,
    name text,
    "desc" text,
    mac macaddr NOT NULL
);


ALTER TABLE public.device OWNER TO root;

--
-- Name: device_id_seq; Type: SEQUENCE; Schema: public; Owner: root
--

CREATE SEQUENCE device_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.device_id_seq OWNER TO root;

--
-- Name: device_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: root
--

ALTER SEQUENCE device_id_seq OWNED BY device.id;


--
-- Name: sample; Type: TABLE; Schema: public; Owner: root; Tablespace: 
--

CREATE TABLE sample (
    id integer NOT NULL,
    "from" timestamp without time zone NOT NULL,
    til timestamp without time zone NOT NULL,
    device_mac macaddr NOT NULL,
    v_1 numeric NOT NULL,
    v_2 numeric NOT NULL,
    i_1 numeric NOT NULL,
    i_2 numeric NOT NULL,
    p_1 numeric NOT NULL,
    p_2 numeric NOT NULL,
    f numeric NOT NULL
);


ALTER TABLE public.sample OWNER TO root;

--
-- Name: sample_id_seq; Type: SEQUENCE; Schema: public; Owner: root
--

CREATE SEQUENCE sample_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.sample_id_seq OWNER TO root;

--
-- Name: sample_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: root
--

ALTER SEQUENCE sample_id_seq OWNED BY sample.id;


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: root
--

ALTER TABLE ONLY device ALTER COLUMN id SET DEFAULT nextval('device_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: root
--

ALTER TABLE ONLY sample ALTER COLUMN id SET DEFAULT nextval('sample_id_seq'::regclass);


--
-- Name: device_addr_key; Type: CONSTRAINT; Schema: public; Owner: root; Tablespace: 
--

ALTER TABLE ONLY device
    ADD CONSTRAINT device_addr_key UNIQUE (addr);


--
-- Name: device_pkey; Type: CONSTRAINT; Schema: public; Owner: root; Tablespace: 
--

ALTER TABLE ONLY device
    ADD CONSTRAINT device_pkey PRIMARY KEY (id);


--
-- Name: mac_unique; Type: CONSTRAINT; Schema: public; Owner: root; Tablespace: 
--

ALTER TABLE ONLY device
    ADD CONSTRAINT mac_unique UNIQUE (mac);


--
-- Name: sample_pkey; Type: CONSTRAINT; Schema: public; Owner: root; Tablespace: 
--

ALTER TABLE ONLY sample
    ADD CONSTRAINT sample_pkey PRIMARY KEY (id);


--
-- Name: device; Type: FK CONSTRAINT; Schema: public; Owner: root
--

ALTER TABLE ONLY sample
    ADD CONSTRAINT device FOREIGN KEY (device_mac) REFERENCES device(mac) MATCH FULL ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;