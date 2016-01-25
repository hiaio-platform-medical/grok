/*
*    Copyright (C) 2016 Grok Image Compression Inc.
*
*    This source code is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This source code is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*
*    This source code incorporates work covered by the following copyright and
*    permission notice:
*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2014, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2014, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux
 * Copyright (c) 2003-2014, Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <stddef.h> /* ptrdiff_t */

	/**
	@file bio.h
	@brief Implementation of an individual bit input-output (BIO)

	The functions in BIO.C have for goal to realize an individual bit input - output.
	*/

	/** @defgroup BIO BIO - Individual bit input-output stream */
	/*@{*/

	/**
	Individual bit input-output stream (BIO)
	*/
	typedef struct opj_bio {
		/** pointer to the start of the buffer */
		uint8_t *start;
		/** pointer to the end of the buffer */
		uint8_t *end;
		/** pointer to the present position in the buffer */
		uint8_t *bp;
		/** temporary place where each byte is read or written */
		uint8_t buf;
		/** coder : number of bits free to write. decoder : number of bits read */
		uint8_t ct;

		bool sim_out;
	} opj_bio_t;

	/** @name Exported functions */
	/*@{*/
	/* ----------------------------------------------------------------------- */
	/**
	Create a new BIO handle
	@return Returns a new BIO handle if successful, returns NULL otherwise
	*/
	opj_bio_t* opj_bio_create(void);
	/**
	Destroy a previously created BIO handle
	@param bio BIO handle to destroy
	*/
	void opj_bio_destroy(opj_bio_t *bio);
	/**
	Number of bytes written.
	@param bio BIO handle
	@return Returns the number of bytes written
	*/
	ptrdiff_t opj_bio_numbytes(opj_bio_t *bio);
	/**
	Init encoder
	@param bio BIO handle
	@param bp Output buffer
	@param len Output buffer length
	*/
	void opj_bio_init_enc(opj_bio_t *bio, uint8_t *bp, uint32_t len);
	/**
	Init decoder
	@param bio BIO handle
	@param bp Input buffer
	@param len Input buffer length
	*/
	void opj_bio_init_dec(opj_bio_t *bio, uint8_t *bp, uint32_t len);
	/**
	Write bits
	@param bio BIO handle
	@param v Value of bits
	@param n Number of bits to write
	*/
	void opj_bio_write(opj_bio_t *bio, uint32_t v, uint32_t n);
	/**
	Read bits
	@param bio BIO handle
	@param n Number of bits to read
	@return Returns the corresponding read number
	*/
	uint32_t opj_bio_read(opj_bio_t *bio, uint32_t n);
	/**
	Flush bits
	@param bio BIO handle
	@return Returns true if successful, returns false otherwise
	*/
	bool opj_bio_flush(opj_bio_t *bio);
	/**
	Passes the ending bits (coming from flushing)
	@param bio BIO handle
	@return Returns true if successful, returns false otherwise
	*/
	bool opj_bio_inalign(opj_bio_t *bio);
	/* ----------------------------------------------------------------------- */
	/*@}*/

	/*@}*/


