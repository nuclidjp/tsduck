//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2020, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------
//
//  Representation of a service_descriptor
//
//----------------------------------------------------------------------------

#include "tsServiceDescriptor.h"
#include "tsDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsTablesFactory.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"service_descriptor"
#define MY_DID ts::DID_SERVICE
#define MY_STD ts::STD_DVB

TS_XML_DESCRIPTOR_FACTORY(ts::ServiceDescriptor, MY_XML_NAME);
TS_ID_DESCRIPTOR_FACTORY(ts::ServiceDescriptor, ts::EDID::Standard(MY_DID));
TS_FACTORY_REGISTER(ts::ServiceDescriptor::DisplayDescriptor, ts::EDID::Standard(MY_DID));


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::ServiceDescriptor::ServiceDescriptor(uint8_t type, const UString& provider, const UString& name) :
    AbstractDescriptor(MY_DID, MY_XML_NAME, MY_STD, 0),
    service_type(type),
    provider_name(provider),
    service_name(name)
{
    _is_valid = true;
}

ts::ServiceDescriptor::ServiceDescriptor(DuckContext& duck, const Descriptor& desc) :
    ServiceDescriptor()
{
    deserialize(duck, desc);
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::ServiceDescriptor::serialize(DuckContext& duck, Descriptor& desc) const
{
    ByteBlockPtr bbp(serializeStart());
    bbp->appendUInt8(service_type);
    bbp->append(duck.encodedWithByteLength(provider_name));
    bbp->append(duck.encodedWithByteLength(service_name));
    serializeEnd(desc, bbp);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::ServiceDescriptor::deserialize(DuckContext& duck, const Descriptor& desc)
{
    _is_valid = desc.isValid() && desc.tag() == _tag && desc.payloadSize() >= 3;

    if (_is_valid) {
        const uint8_t* data = desc.payload();
        size_t size = desc.payloadSize();
        service_type = data[0];
        data++; size--;
        duck.decodeWithByteLength(provider_name, data, size);
        duck.decodeWithByteLength(service_name, data, size);
        _is_valid = size == 0;
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::ServiceDescriptor::DisplayDescriptor(TablesDisplay& display, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    DuckContext& duck(display.duck());
    std::ostream& strm(duck.out());
    const std::string margin(indent, ' ');

    if (size >= 1) {
        // Service type.
        uint8_t stype = *data;
        data += 1; size -= 1;
        strm << margin << "Service type: " << names::ServiceType(stype, names::FIRST) << std::endl;

        // Provider and service names (data and size are updated by FromDVBWithByteLength).
        const UString provider(duck.decodedWithByteLength(data, size));
        const UString service(duck.decodedWithByteLength(data, size));
        strm << margin << "Service: \"" << service << "\", Provider: \"" << provider << "\"" << std::endl;
    }

    display.displayExtraData(data, size, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::ServiceDescriptor::buildXML(DuckContext& duck, xml::Element* root) const
{
    root->setIntAttribute(u"service_type", service_type, true);
    root->setAttribute(u"service_provider_name", provider_name);
    root->setAttribute(u"service_name", service_name);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

void ts::ServiceDescriptor::fromXML(DuckContext& duck, const xml::Element* element)
{
    _is_valid =
        checkXMLName(element) &&
        element->getIntAttribute<uint8_t>(service_type, u"service_type", true) &&
        element->getAttribute(provider_name, u"service_provider_name", true) &&
        element->getAttribute(service_name, u"service_name", true);
}
