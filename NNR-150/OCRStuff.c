unsigned char copy_ocr_data[20000];
unsigned char dst[30*30];
unsigned char FeatureInBytes[30*30];

int      ir_freq[256];

#define	MAX_CHAR_X_POS	20
int OCR_x_pos[MAX_CHAR_X_POS];
int OCR_x_len[MAX_CHAR_X_POS];

int ORG_x_pos[MAX_CHAR_X_POS] = { 0, 8, 16, 24, 32, 40, 48, 54, 62 };
int ORG_x_len[MAX_CHAR_X_POS] = { 6, 6,  6,  6,  6,  6,  6,  6,  6 };
int ORG_y_len[MAX_CHAR_X_POS] = { 13, 13, 13, 13, 13, 13, 13, 13, 13 };

int OCR_max_length;

int OCR_pos_cnt = 9;

int GetLengthInBytesForNNROCR(int width, int height)
{
	return height*(width/2);
}

// MakeFeatureForNNROCR_NoScale
// MakeFeatureForNNROCR�� ���� �������� 1/2�� ����ϴ� ����� ������ �ִµ�, ���� ���� ��쿡 �ø��� ������ �����̾
// ȸ���� �Ŀ��� ���� �������� ����ϸ� ����ϴ�. ���� ���� �������� ������� �ʴ� �Լ��� �������.
int MakeFeatureForNNROCR_NoScale(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height)
{
	// 0~255 ������ ���� �������� �ٲ۴�.
	int x, y;
	int start_pos = 0;
	int minv = 255;
	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			int c = srcbuf[y*width + x];
			if(c < minv)
				minv = c;
			dstbuf[start_pos++] = c;
		}
	}
	if(255 == minv)
		return start_pos; // ��� ���� 255�� ���. �� ĭ.

	for(x = 0; x < start_pos; x++)
		dstbuf[x] = ((dstbuf[x]-minv)*255) / (255-minv); // minv�� 255 ���̰� 255�� Ȯ��ȴ�. ���� �ִ����� �𸣰ڴ�.

	return start_pos;
}

// ScaleImageXY
// Scale Factor�� �༭ ��Ҹ� �����Ѵ�.
// ����� Y�� �������� �������� �ʾҴ�. dschae 20121225
// ���� �ּ�ġ�� ���� �����ϸ��ϴ� ��ɵ� ����.
int ScaleImageXY(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height, int xscale, int yscale)
{
	// 0~255 ������ ���� �������� �ٲ۴�.
	int x, y;
	int cnt = 0;
	int sum = 0;
	int start_pos = 0;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			sum += srcbuf[y*width + x];
			cnt++;
			if(cnt >= xscale) {
				dstbuf[start_pos++] = sum / xscale;
				cnt = 0;
				sum = 0;
			}
		}
	}

	return start_pos;
}

//
// NNR ���� ���α׷��� �׳� 128�� ���� 128�� �����⸸ �ϴ� �� ����.
// �׷��� ���� ��ƾ�� ����� ����.
int SimpleMakeFeatureForNNROCR(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height)
{
	int x, y;
	int start_pos = 0;
	int minv = 255;
	for(y = 0; y < height; y++) {
		for(x = 0; x < width/2; x++) {
			int c = (srcbuf[y*width + x*2] + srcbuf[y*width + x*2 + 1]) / 2;
			if(c < minv) minv = c;
			dstbuf[start_pos++] = c;
		}
	}
	return start_pos;
}

int MakeFeatureForNNROCR(unsigned char* srcbuf, unsigned char* dstbuf, int width, int height)
{
	// 0~255 ������ ���� �������� �ٲ۴�.
#if 1
	int x, y;
	int start_pos = 0;
	int minv = 255;
	for(y = 0; y < height; y++) {
		for(x = 0; x < width/2; x++) {
			int c = (srcbuf[y*width + x*2] + srcbuf[y*width + x*2 + 1]) / 2;
			if(c < minv) minv = c;
			dstbuf[start_pos++] = c;
		}
	}
#if 0 // NNR ���� ���α׷��� �׳� 128�� ���� 128�� �����⸸ �ϴ� �� ����.
	for(x = 0; x < start_pos; x++)
		dstbuf[x] = ((dstbuf[x]-minv)*255) / (255-minv);
	return start_pos;
#endif
#else
	int x, y;
	int start_pos = 0;
	for(y = 0; y < height; y++) {
		dstbuf[y] = 127;
		for(x = 0; x < width; x++) {
			if(srcbuf[y*width + x] != 255) {
				dstbuf[start_pos+y] = (x*255) / width;
				break;
			}
		}
	}

	start_pos += height;
	for(y = 0; y < height; y++) {
		dstbuf[start_pos+y] = 127;
		for(x = 0; x < width; x++) {
			if(srcbuf[y*width + width-1-x] != 255) {
				dstbuf[start_pos+y] = (x*255) / width;
				break;
			}
		}
	}

	start_pos += height;
	for(y = 0; y < height; y++) {
		int cnt = 0;
		for(x = 0; x < width; x++) {
			if(srcbuf[y*width + x] != 255) {
				cnt++;
			}
		}
		dstbuf[start_pos+y] = (cnt*255) / width;
	}

	start_pos += height;
	for(x = 0; x < width; x++) {
		for(y = 0; y < height; y++) {
			if(srcbuf[y*width + x] != 255) {
				dstbuf[start_pos+x] = (y*255)/height;
				break;
			}
		}
	}

	start_pos += width;
	for(x = 0; x < width; x++) {
		int cnt = 0;
		for(y = 0; y < height; y++) {
			if(srcbuf[y*width + x] != 255) {
				cnt++;
			}
		}
		dstbuf[start_pos+x] = (cnt*255) / height;
	}
#endif
	return start_pos + width;
}

void RemoveUpperBlank(unsigned char* image, int width, int height)
{
	int x, y;
	int blankline = -1;
	for( y = 0; y < height; y++) {
		int xsum = 0;
		for( x = 0; x < width; x++) {
			xsum += image[y*width + x];
		}
		if(xsum == width*255)
			blankline = y;
		else
			break;
	}

	if(blankline == -1)
		return;

	blankline += 1;
	for( y = 0; y < height-blankline; y++) {
		for(x = 0; x < width; x++) {
			image[y*width + x] = image[(y+blankline)*width + x];
		}
	}
	for( ; y < height; y++) {
		for(x = 0; x < width; x++) {
			image[y*width + x] = 255;
		}
	}
}

int ir_x_sum[60];
int OCR_y_pos_0, OCR_y_pos_1;
unsigned short OCR_ImgWidth;
unsigned short OCR_ImgHeight;

int FindVerticalWindow(unsigned char* copy_ir_data, int xsize, int ysize, int vwindow)
{
	int x, y, m;
	int yp0, yp1;
	int miny = -1;
	int minv = 255 * ysize * xsize;
	int ir_m_sum;

	OCR_y_pos_0 = 0;
	OCR_y_pos_1 = 0;

	OCR_ImgWidth = xsize;
	OCR_ImgHeight = ysize;

	for(y = 0; y < ysize; y++) {
		ir_x_sum[y] = 0;
		for( x = 0; x < xsize; x++) {
			ir_x_sum[y] += copy_ir_data[y*xsize + x];
		}
	}

	// �̵��ϸ� vwindow ��ġ�� �ջ��Ѵ�.
	for(y = 0; y < ysize-vwindow; y++) {
		ir_m_sum = 0;
		for(m = 0; m < vwindow; m++) {
			ir_m_sum += ir_x_sum[y + m];
		}
		if(ir_m_sum < minv) {
			// ���� �ּҰ��� y�� ���Ѵ�.
			minv = ir_m_sum;
			miny = y;
		}
	}
	
	// �� ���� �ִ� �ε����� �̵��Ѵ�.
	yp0 = miny;
	yp1 = miny + vwindow;
	// ���ٸ���
	while(yp0 < ysize) {
		if(ir_x_sum[yp0] < 255 * xsize)
			break;
		yp0++;
	}
	// �Ʒ���
	while(yp1 > 0) {
		if(ir_x_sum[yp1-1] < 255 * xsize)
			break;
		yp1--;
	}

	if(yp0 > yp1) {
		yp0 = yp1 = 0;
	}
	
	OCR_y_pos_0 = yp0;
	OCR_y_pos_1 = yp1;

	return yp1 - yp0;
}

int FindVerticalWindowSE(unsigned char* copy_ir_data, int xsize, int ysize, int vwindow, int* ys, int* ye)
{
	int x, y, m;
	int yp0, yp1;
	int miny = -1;
	int minv = 255 * ysize * xsize;
	int ir_m_sum;

	OCR_ImgWidth = xsize;
	OCR_ImgHeight = ysize;

	for(y = 0; y < ysize; y++) {
		ir_x_sum[y] = 0;
		for( x = 0; x < xsize; x++) {
			ir_x_sum[y] += copy_ir_data[y*xsize + x];
		}
	}

	// �̵��ϸ� vwindow ��ġ�� �ջ��Ѵ�.
	for(y = 0; y < ysize-vwindow; y++) {
		ir_m_sum = 0;
		for(m = 0; m < vwindow; m++) {
			ir_m_sum += ir_x_sum[y + m];
		}
		if(ir_m_sum < minv) {
			// ���� �ּҰ��� y�� ���Ѵ�.
			minv = ir_m_sum;
			miny = y;
		}
	}
	
	// �� ���� �ִ� �ε����� �̵��Ѵ�.
	yp0 = miny;
	yp1 = miny + vwindow;
	// ���ٸ���
	while(yp0 < ysize) {
		if(ir_x_sum[yp0] < 255 * xsize)
			break;
		yp0++;
	}
	// �Ʒ���
	while(yp1 > 0) {
		if(ir_x_sum[yp1-1] < 255 * xsize)
			break;
		yp1--;
	}

	if(yp0 > yp1) {
		yp0 = yp1 = 0;
	}
	
	*ys = yp0;
	*ye = yp1;

	return yp1 - yp0;
}

unsigned short ir_y_cnt[800];
unsigned short ir_y_sum[800];
unsigned int   ir_y_wgt[800]; // weight - ���� ������ ����.
/*
	FindHorizontalWindows(
	unsigned char* copy_ir_data - ������.
	int img_width, int xwindow, 
	int* max_len, - maximum length of window
	int max_cnt - maximum number of supposed charcters.
	int margin - minimum pixels required to be a character. - �� ���ڸ� �̷�� �� �ּ� �ȼ� ��
	int x_limit)
*/
int FindHorizontalWindows(unsigned char* copy_ir_data, int img_width, int xwindow, 
	int* max_len, int max_cnt, int margin, int x_limit, int method)
{
	int x, y;
	int char_pos_cnt = 0;
	int tsum = 0;
	int length = 0;
	int old_x_pos = 0;
	int y_size = OCR_y_pos_1 - OCR_y_pos_0;
	int max_length = 0;
	int no_bit = 0;

	int vbar = ((xwindow*y_size)*margin)/1000;
	if(img_width > 800)
		return 0;

	for(x = 0; x < img_width; x++) {
		ir_y_cnt[x] = 0;
		ir_y_sum[x] = 0;
		ir_y_wgt[x] = 0;
		for(y = OCR_y_pos_0; y < OCR_y_pos_1; y++) {
			unsigned char c = copy_ir_data[y*img_width+ x];
			if(c < 255) ir_y_cnt[x]++;
			ir_y_sum[x] += c;
		}
	}

	if(method == 1) {
		unsigned int weight = 0;
		for(x = 1; x < xwindow; x++) {
			weight += ir_y_sum[x];
			ir_y_wgt[x] = xwindow * y_size * 255;
		}
		for(x = xwindow; x < img_width;) {
			weight += ir_y_sum[x];
			ir_y_wgt[x] = weight;
			x++;
			weight -= ir_y_sum[x-xwindow];
		}
	}

	for(x = 0; x < img_width; x++) {
		int too_long = 0;
		int seperate = 0;

		if(ir_y_cnt[x] > 0) {
			if(length == 0)
				old_x_pos = x;
			length++;
			tsum += ir_y_cnt[x];
			no_bit = 0;
		} else {
			no_bit++;
			if(no_bit > 1) {
				length = 0; // �������� �ǽɵǰ� �ִ� ���� �κ��� �����Ѵ�.
				tsum = 0;
			}
		}

		if(length > (xwindow*3)/2) {
			too_long = 1;
			if(char_pos_cnt > 0 && OCR_x_pos[char_pos_cnt-1]+xwindow > old_x_pos && ir_y_cnt[x]) {
				// ���� ���� ��ġ�κ��� �ʹ� ������ ���۵Ǿ��ٸ�.
				old_x_pos++;
				length--;
				too_long = 0;
			}
		}
		
		if((length > 0 && ir_y_cnt[x] == 0) || too_long) {
			if(length > xwindow) {
				int last_x_pos = x - 1;
				switch(method){
				case 0 : // using count of under 255 pixels
					while(length > xwindow) {
						if(ir_y_cnt[old_x_pos]+ir_y_cnt[old_x_pos+1]+ir_y_cnt[old_x_pos+2] > ir_y_cnt[last_x_pos]+ir_y_cnt[last_x_pos-1]+ir_y_cnt[last_x_pos-2]) {
							// ���ڰ� �������� �� ġ��ģ�ٸ�
							tsum -= ir_y_cnt[last_x_pos];
							last_x_pos--;
						} else {
							tsum -= ir_y_cnt[old_x_pos];
							old_x_pos++;
						}
						length--;
					}
					x = old_x_pos + length + 1;
					break;
				case 1 : // ���԰� �ּ��� ��ġ�� ã�´�.
					{
						int movx;
						tsum = 0;
						last_x_pos = old_x_pos+xwindow;
						for(movx = old_x_pos+xwindow; movx < x; movx++) {
							if(ir_y_wgt[movx] < ir_y_wgt[last_x_pos])
								last_x_pos = movx;
						}
						old_x_pos = last_x_pos - xwindow;
						for(movx = old_x_pos; movx < last_x_pos; movx++)
							tsum += ir_y_cnt[movx];
						length = xwindow;
						x = old_x_pos + length + 1;
					}
				}
			}

			if(tsum > vbar)
				seperate = 1;
			else
				seperate = 2; // �ϴ� �Ʒ� L315���� �ٽ� ������� �ʴ´ٸ� �������鼭 �����Ǵ� ���̴�.
			//	|| length > (xwindow/3)) // ũ�Ⱑ ū �͸� �и� ����̴�.
		}

		if(length > 0 && ir_y_cnt[x] == 0) { // L315.
			if(length < xwindow-2 && ir_y_cnt[x+1] && ir_y_cnt[x+2]) {
				// �и� ��������� ��¦ ������ ���� ������ �ش�.
				seperate = 0;
				length++;
			}
		}

		switch(seperate){
		case 1 : // ���ڷ� ����Ѵ�.
			OCR_x_pos[char_pos_cnt] = old_x_pos;
			OCR_x_len[char_pos_cnt] = length;
			char_pos_cnt++;
			if(x < old_x_pos+length)
				x = old_x_pos+length;

			if(length > max_length)
				max_length = length;
			length = 0;
			tsum = 0;
			break;
		case 2 : // L315���� ���ڰ� �ٽ� ������� �ʾҴٸ� ���� ����� �����̴�.
			length = 0;
			tsum = 0;
		}
		if(char_pos_cnt > max_cnt)
			break;
	}

#if 0 // ������ ��� �����Ѵ�.
	while(OCR_x_pos[char_pos_cnt-1] - OCR_x_pos[0] > x_limit) {
		// �� ���� ���� - �̹��� ������� ���� ��¥ ������ �ν��ϴ� ��츦 �����ϱ� ����
		if(OCR_x_pos[char_pos_cnt-1] - OCR_x_pos[char_pos_cnt-2] < OCR_x_pos[1]-OCR_x_pos[0]) {
			short xp;
			for(xp = 1; xp < char_pos_cnt; xp++) {
				OCR_x_pos[xp-1] = OCR_x_pos[xp];
				OCR_x_len[xp-1] = OCR_x_len[xp];
			}
		}
		char_pos_cnt--;
	}
#endif
	
	*max_len = max_length;
	return char_pos_cnt;
}

void RemoveMetalThread(unsigned char* data, int width, int height, int metal)
{
	int x, y;
	for(x = 0; x < width; x++) {
		int sum = 0;
		for(y = 0; y < height; y++)
			sum += data[width*y + x];
		sum /= height;
		if(sum < metal) {
			for(y = 0; y < height; y++)
				data[width*y + x] = 255;
		}
	}
}

int GetThresholdByPixels(int* freq, unsigned char* data, int width, int height, int count)
{
	int Sum = 0;
	int i;
	int size = width * height;

	memset(freq, 0, sizeof(int)*256);

	for(i = 0; i < size; i++) {
		unsigned char c = data[i];
		freq[c]++;
	}

	for( i = 0; i < 256 && Sum < count; i++) {
		Sum += freq[i];
	}
	return i-1;
}

// GetThresholdByPixels�� ���� ����ε�, �ٸ� ��� ������ ���� ũ�Ⱑ OCR_y_pos_0, OCR_y_pos_1�� ��������.
int GetThresholdByPixelsYY(int* freq, unsigned char* data, int width, int height, int count)
{
	int Sum = 0;
	int i;
	int x, y;
	//int size = width * (OCR_y_pos_1 - OCR_y_pos_0); //ryuhs74@20121212 - Not USED ���� �ּ�ó��

	memset(freq, 0, sizeof(int)*256);

	for(y = OCR_y_pos_0; y < OCR_y_pos_1; y++) {
		for(x = 0; x < width; x++) {
			unsigned char c = data[y * width + x];
			freq[c]++;
		}
	}

	for( i = 0; i < 256 && Sum < count; i++) {
		Sum += freq[i];
	}
	return i-1;
}

unsigned char _srcbuf[100*100];

#if 1 // 20121224 �� �Լ��� ���� ���ļ� ������ �����Ѵ�.
void RemoveSidePixels(unsigned char* srcbuf, int old_w, int old_h, int new_w, int new_h)
{
	int x, y;
	int copy_start_x = 0;
	int copy_start_y = 0;
	int copy_end_x = old_w;
	int copy_end_y = old_h;

	memset(_srcbuf, 255, new_w*new_h);
	
	// ���� ���� �̵����� ó���Ѵ�.
	while(copy_start_y < old_h) {
		for(x = 0; x < old_w; x++) {
			if(srcbuf[copy_start_y*old_w + x] != 255)
				break;
		}
		if(x < old_w)
			break;// ���� ���� �ִٸ� �̵� ���� ��
		copy_start_y++; // �̵� ������ ����
	}

	// �̹����� �¿� �̵��̴�.
	while(copy_start_x < old_w) {
		for(y = copy_start_y; y < old_h; y++) {
			if(srcbuf[y*old_w + copy_start_x] != 255)
				break;
		}
		if(y < old_h) // ���� ���� �ִٸ� �̵� ���� ��
			break;
		copy_start_x++; // �̵� ������ ����
	}

	// �� ���� ���縦 �����ϴµ�...
	{
		int d_x = copy_end_x - copy_start_x;
		while(d_x > new_w) {
			int l_cnt = 0;
			int r_cnt = 0;
			for(y = copy_start_y; y < copy_end_y; y++) {
				if(srcbuf[y*old_w + copy_start_x] != 255) l_cnt++;
				if(srcbuf[y*old_w + copy_end_x-1] != 255) r_cnt++;
			}
			if(r_cnt > l_cnt) copy_start_x++; // ������ ���� �� ������ �������� ����
			else copy_end_x--; // �ƴϸ� �������� ����
			d_x--;
		}
	}

	// �� ���� ���縦 �����ϴµ�...
	{
		int d_y = copy_end_y - copy_start_y;
		while(d_y > new_h) {
			int t_cnt = 0;
			int b_cnt = 0;
			for(x = copy_start_x; x < copy_end_x; x++) {
				if(srcbuf[copy_start_y*old_w + x] != 255) t_cnt++;
				if(srcbuf[(copy_end_y-1)*old_w + x] != 255) b_cnt++;
			}
			if(b_cnt > t_cnt) copy_start_y++; // �Ͽ� ���� �� ������ �󿡼� ����
			else copy_end_y--; // �ƴϸ� �Ͽ��� ����
			d_y--;
		}
	}

	{
		int yy = 0;
		for(y = copy_start_y; y < copy_end_y; y++) {
			int xx = 0;
			for(x = copy_start_x; x < copy_end_x; x++) {
				_srcbuf[yy * new_w + xx] = srcbuf[y*old_w + x];
				xx++;
			}
			yy++;
		}
	}

	memcpy(srcbuf, _srcbuf, new_w * new_h);
}
#else
void RemoveSidePixels(unsigned char* srcbuf, int old_w, int old_h, int new_w, int new_h)
{
	int cnt[64];
	int x, y;
	int mov_h;

	// ���� ������ ó���Ѵ�.
	if(old_w > new_w) {
		int delete_x = 0;
		for( x = 0; x < old_w; x++) {
			cnt[x] = 0;
			for( y = 0; y < old_h; y++) {
				cnt[x] += srcbuf[y*old_w + x];
			}
		}

		delete_x = 0;
		mov_h = old_w;
		while((mov_h-delete_x) > new_w) {
			if(cnt[delete_x] > cnt[mov_h-1])
				delete_x++;
			else
				mov_h--;
		}
		
		for( y = 0; y < old_h; y++) {
			for( x = delete_x; x < mov_h; x++) {
				_srcbuf[y*new_w + x-delete_x] = srcbuf[y*old_w + x];
			}
		}
		memcpy(srcbuf, _srcbuf, new_w*new_h);
	}

	// ���̸� ó���Ѵ�.
	if(old_h > new_h) {
		int delete_y = 0;

		for(y = 0; y < old_h; y++) {
			cnt[y] = 0;
			for(x = 0; x < new_w; x++)
				cnt[y] += srcbuf[(y*new_w) + x];
		}

		mov_h = old_h;
		while(mov_h > new_h) {
			if(cnt[delete_y] > cnt[old_h-1])
				delete_y++; // ���� ������ ������ �� �� �������.
			else
				old_h--; // �Ʒ����� �������.
			mov_h--; // �� ���� ����� ����.
		}
		// delete_y���� �ؼ� �����Ѵ�.
		if(delete_y > 0) {
			for(y = delete_y; y < old_h; y++) {
				int yy = y - delete_y;
				for(x = 0; x < new_w; x++) {
					srcbuf[yy*new_w + x] = srcbuf[y*new_w + x];
				}
			}
		}
	}
}
#endif

//
// ���� �ػ󵵴� �׻� ��Ȯ�ϰ� �����Ǵµ�, �� �ȼ� ���̰ų�
// �� �ȼ� ���� ���̶�� ������� �����ϰ� �����Ѵ�.
// �׸�? �� �� ���� �پ��ִ� �͵� ������� �����ϰ� �����Ѵ�.
//
// dschae@20121224
//
unsigned char _tstbuf[30*30];

void RemoveHSingleDots(unsigned char* srcbuf, int old_w, int old_h)
{
	unsigned short x, y;
	int base;

	base = 0;
	for(y = 0; y < old_h; y++) {
		for(x = 0; x < old_w; x++) {
			unsigned char pv = 0;
			if(srcbuf[base + x] != 255) {
				pv = 1;
				if(y > 0) {
					if(srcbuf[base - old_w + x] != 255) pv++;
				}
				if(y < old_h-1) {
					if(srcbuf[base + old_w + x] != 255) pv++;
				}
			}
			_tstbuf[base + x] = pv;
		}
		base += old_w;
	}
	
	base = 0;
	for(y = 0; y < old_h; y++) {
		unsigned short last_x;
		unsigned short count = 0; // ���ӵ� ���� �ֺ� �� ���� ��
		for(x = 0; x < old_w; x++) { // '<='�� ���ϴ� ������ ������ �ȼ����� �����ؼ� �˻��ϱ� �����̴�.
			unsigned char pv = _tstbuf[base + x];
			if(count) { // �������� ���� �־��ٸ�
				if(pv == 0) { // ���� ������
					if(count < 5) {
						unsigned short xx; for(xx = last_x; xx < x; xx++) srcbuf[base+xx] = 255;
					}
					count = 0;
				} else {
					count += pv; // �ֺ� ���� �������� ���� �ջ� �ȴ�.
				}
			} else { // �������� ���� �����ٸ�
				count = pv; // �� ���� �Ʒ� ���� ���� �� �ִٸ� 3�� �ȴ�.
				last_x = x;
			}
		}
		if(count && count < 5) {
			unsigned short xx; for(xx = last_x; xx < x; xx++) srcbuf[base+xx] = 255;
		}
		base += old_w;
	}
}

int CountUnderThreshold(int threshold, unsigned char* w_data, unsigned char* copy_w_data, int width, int height)
{
	int x, y;
	int count = 0;

	for( y = 0; y < height; y++) {
		for( x = 0; x < width; x++) {
			unsigned char c;
			c = w_data[y*width + x];
			if(c > threshold)
				c = 255;
			else {
				count++;
//				c = 0;
			}
			copy_w_data[y*width + x] = c;
		}
	}

	return count;
}

#ifndef DIR_FF
#define DIR_FF				0
#define DIR_FB				1
#define DIR_BF				2
#define DIR_BB				3
#endif

void AdjustImageDirection(unsigned short Dir, unsigned char* w_data, unsigned char* ir_data, int width, int height)
{
	int x, y;
	int right, left, high, low;
	left = 0; right = width-1;

	// ���� ������� �ݻ�,���� ������ �ʰ�, DIR_xB�� ��쿡 ���Ѵ�.
	if(Dir == DIR_FB || Dir == DIR_BB) {
		if(w_data != 0) {
			low = 0; high = height-1;
			while(high > low) {
				for( x = 0; x < width; x++) {
					unsigned char uc = w_data[high*width + x];
					w_data[high*width + x] = w_data[low*width + x];
					w_data[low*width + x] = uc;
				}
				high--; low++;
			}
		}
		if(ir_data != 0) {
			low = 0; high = height-1;
			while(high > low) {
				for( x = 0; x < width; x++) {
					unsigned char uc = ir_data[high*width + x];
					ir_data[high*width + x] = ir_data[low*width + x];
					ir_data[low*width + x] = uc;
				}
				high--; low++;
			}
		}
	}

	// �ݻ� �¿� ������� DIR_FF || Dir == DIR_BF || Dir == DIR_BB
	if(w_data != 0) {
		if(Dir == DIR_FF || Dir == DIR_BF) {
			left = 0; right = width-1;
			while(left < right) {
				for( y = 0; y < height; y++) {
					unsigned char uc = w_data[y*width + left];
					w_data[y*width + left] = w_data[y*width + right];
					w_data[y*width + right] = uc;
				}
				left++; right--;
			}
		}
	}

	// IR �¿� ������� DIR_BB, DIR_FF�� �� �Ѵ�. ���� EUR, TRY���� �°� ������ �Ǵµ�, �ٸ� ��ȭ�� �� �� ���� �ִ�.
	if(ir_data != 0) {
		if(Dir == DIR_FF || Dir == DIR_BB){
			left = 0; right = width-1;
			while(left < right) {
				for( y = 0; y < height; y++) {
					unsigned char uc = ir_data[y*width + left];
					ir_data[y*width + left] = ir_data[y*width + right];
					ir_data[y*width + right] = uc;
				}
				left++; right--;
			}
		}
	}
}

void SwapImageVertical(unsigned char* _data, int width, int height)
{
	int x;//, y; //ryuhs74@20121212 - Not USED ���� �ּ�ó��
	int /*right, left,*/ high, low; //ryuhs74@20121212 - Not USED ���� �ּ�ó��
	//left = 0; right = width-1; //ryuhs74@20121212 - Not USED ���� �ּ�ó��

	// ���� ������� �ݻ�,���� ������ �ʰ�, DIR_xB�� ��쿡 ���Ѵ�.
	low = 0; high = height-1;
	while(high > low) {
		for( x = 0; x < width; x++) {
			unsigned char uc = _data[high*width + x];
			_data[high*width + x] = _data[low*width + x];
			_data[low*width + x] = uc;
		}
		high--; low++;
	}
}

void SwapImageHorizontal(unsigned char* _data, int width, int height)
{
	int y;
	int right, left;//, high, low; //ryuhs74@20121212 - Not USED ���� �ּ�ó��
	left = 0; right = width-1;

	// �¿� ������� DIR_BB, DIR_FF�� �� �Ѵ�.
	left = 0; right = width-1;
	while(left < right) {
		for( y = 0; y < height; y++) {
			unsigned char uc = _data[y*width + left];
			_data[y*width + left] = _data[y*width + right];
			_data[y*width + right] = uc;
		}
		left++; right--;
	}
}

//
// MakeOneImage - �Ϸù�ȣ �̹��� ��ü���� �� ���� ��ŭ�� �̹����� �����´�.
// unsigned char* dst : ���� �̹����� �޾ƿ� ����
// unsigned char* copy_ir_data : �Ϸù�ȣ�� ����ִ� ���� �̹���
// int i : ������ ���� ��ȣ
// int* char_x_pos : ������ ���� �̹����� �����ϴ� ���� X ��ǥ�� ��� �ִ� ���� �迭
// int img_xsize : ���� �̹����� ���� ũ��
// int y_start : ���� �̹��� ������ ���ڰ� �����ϴ� ���� ��ġ
// int x_window : ������ �̹����� ���ϴ� ���� ũ��
// int y_window : ���� ������ ���� ũ��
// int SaveHeight : ������ �̹����� ���ϴ� ���� ũ��
// int SrcWidth : ���� ������ ���� ũ��
//-- �Ű����� ������ ��쿡 ������ ����� ũ�Ⱑ �ڼ��� ������ �ʱ� ���� �������� ������ �߰��� ������ �Ǿ��ֱ� �����̴�.
// ����, y_start�� �Ű������� �ް�, char_x_pos�� ���������� �޴� ������ �׳� �׷��� ���ߵǾ��� �����̴�.
// �� �κ��� �������� �ʴ� ������ �⺻�� ���ߵ� �ٸ� ������ �ø��� ��ɶ����̴�.
//
#define	TEST_BLACK_WHITE	1

void MakeOneImage(unsigned char* dst, unsigned char* copy_ir_data, int i, int* char_x_pos, int img_xsize, int y_start, int x_window, int y_window, int SaveHeight, int SrcWidth)
{
	int x, y;
	int x_start = char_x_pos[i];
	for( y = 0; y < y_window; y++) {
		int yy = y_start + y;
		int yx = y * SrcWidth;
		for( x = 0; x < SrcWidth; x++) {
#if TEST_BLACK_WHITE
			// �� �ٴ��� �ƴ� ���� ���� ��İ� �Ѵ�.
			unsigned char c = copy_ir_data[yy*img_xsize + x_start + x];
			if(c != 255) c = 0; // ����� �ƴϸ� ���� ��İ�.
			dst[yx + x] = c;
#else
			dst[yx + x] = copy_ir_data[yy*img_xsize + x_start + x];
#endif
		}
	}

	RemoveHSingleDots(dst, SrcWidth, y_window);
	RemoveSidePixels(dst, SrcWidth, y_window, x_window, SaveHeight);
}

void MakeOneImageEx(unsigned char* dst, unsigned char* src_data, int src_img_width, int x_pos, int x_len, int y_start, int y_end, int dst_width, int dst_height)
{
	int x, y;
	int y_size = y_end-y_start;

	memset(dst, 255, x_len * y_size);
	for( y = 0; y < y_size; y++) {
		for( x = 0; x < x_len; x++) {
			dst[y*x_len+ x] = src_data[(y_start+y)*src_img_width + (x_pos+x)];
		}
	}

	RemoveSidePixels(dst, x_len, y_size, dst_width, dst_height);
}

void OCR_GetImage(unsigned char* dst, unsigned short lx, unsigned short ly)
{
	unsigned short x, y;

	for(y = 0; y < ly; y++) {
		unsigned short xp_dst = y * lx;
		unsigned short xp_src = (OCR_y_pos_0+y) * OCR_ImgWidth;
		for(x = 0; x < lx && x < OCR_ImgWidth; x++) {
			dst[xp_dst + x] = copy_ocr_data[xp_src + x];
		}
		for( ; x < lx; x++) {
			dst[xp_dst + x] = 255;
		}
	}
}

int FindHorizontalWindowsEx(unsigned char* copied_data, int img_width, int max_cnt, int* x_pos, int *x_len, int *y_len, int* l_move, int* r_move)
{
	int x, y, si;
	int min_x_pos = 0;
	int y_size = OCR_y_pos_1 - OCR_y_pos_0;
	int min_value = img_width * y_size * 255;
	int total_length = 0;

	// �ϴ� total_length ��ŭ�� X �̹��� ������ ã�´�.
	// �׷��� ���ؼ��� ���� ���� �迭�� ���Ѵ�.
	for(x = 0; x < img_width; x++) {
		ir_y_cnt[x] = 0;
		ir_y_sum[x] = 0;
		ir_y_wgt[x] = 0;
		for(y = OCR_y_pos_0; y < OCR_y_pos_1; y++) {
			unsigned char c = copied_data[y*img_width+ x];
			if(c < 255) ir_y_cnt[x]++;
			ir_y_sum[x] += c;
		}
	}

	// �̵� ���� �ּҰ� �Ǵ� ��ġ�� ã�´�.
	total_length = x_pos[max_cnt-1] + x_len[max_cnt-1]; // �̰� �����̴� x window�� ũ���̴�.
	for(x = 0; 	x < img_width-total_length; x++) {
		int tmp_sum = 0;
		for(si = 0; si < max_cnt; si++) {
			int xx;
			int xs = x_pos[si]+x;
			int xe = xs + x_len[si];
			for(xx = xs; xx < xe; xx++ )
				tmp_sum += ir_y_sum[xx];
		}
		if(tmp_sum < min_value) {
			min_x_pos = x;
			min_value = tmp_sum;
		}
	}

	// �־��� ��ġ���� �� ��� �� �ȼ� ���� �̵��� �� �ֵ��� ���Ѵ�.
	for(si = 0; si < max_cnt; si++) {
		int pos = min_x_pos + x_pos[si];
		int len = x_len[si];
/** �̵��ϰ� �ߴ��� ����� �� �� ����. 20120725�� ���ƹ���.
		if(ir_y_sum[pos-1] < ir_y_sum[pos+len-1]) // ���� �ٱ� �ȼ� �ϳ��� ���� ������ ������ ��Ӵٸ� �������� �ϳ� �̵�.
			pos--;
**/
		if(r_move[si]) { // ���� �̵��� ����� �Ǵ� ���ڶ��
			if(ir_y_sum[pos] > ir_y_sum[pos+len]) // ������ �ٱ� �ȼ� �ϳ��� ���� �������� ��Ӵٸ� �������� �ϳ� �̵�
				pos++;
		}
		OCR_x_pos[si] = pos;
		OCR_x_len[si] = len;
	}

	return max_cnt;
}

