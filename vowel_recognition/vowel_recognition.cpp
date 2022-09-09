// vowel_recognition.cpp : Defines the entry point for the console application.
//

									/*
									SPEECH PROCESSING ASSIGNMENT-3
										VOWEL RECOGNITION

									NAME: DARSHIKA VERMA
									ROLL_NO:214101014 
									*/

//header files to be included
#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "float.h"


double* Compute_Ri(double *sample_data,double *R_ref)     //function to calculate the value of Ri.
{
	int i=0,j=0;
	double sum=0.0;
	for(i=0;i<=12;i++)
	{
		sum=0.0;
		for(j=0;j<320-i;j++)
		{
			sum=sum+(sample_data[j]*sample_data[j+i]);
		}
		R_ref[i]=sum;
	}
	return R_ref;
}
double* Compute_Ai(double *A_ref,double *R)					//function to calculate the value of Ai.
{
	double alpha[13][13], k[13], E[13] ;
	double sum=0.0;
	int i=0,j=0;

	E[0]=R[0];
	for(i=1;i<=12;i++)
	{
		
		if(i==1)
		{
			k[1]=R[1]/R[0];
		}
		else
		{
			sum=0.0;
			for(j=1;j<=i-1;j++)
			{
				sum=sum+(alpha[i-1][j]*R[i-j]);
			}
			k[i]=((R[i]-sum)/E[i-1]);
		}
		alpha[i][i]=k[i];
		for(int j=1; j<=i-1; j++){
            alpha[i][j] = alpha[i-1][j] - (k[i] * alpha[i-1][i-j]);
        }
        E[i] = (1-(k[i] * k[i])) * E[i-1];
	}
		
	for(int j=1;j<=12;j++)
	{
		A_ref[j]=alpha[12][j];
	}
	return A_ref;
}

double* Compute_Ci(double* C_ref,double* A,double* R)		//function to calculate the value of Ci.
{
	int i=0,j=0;
	double sum=0.0;

	C_ref[0]= log(R[0]*R[0]);
	for(i=1;i<=12;i++)
	{
		sum=0.0;
		for(j=1;j<=i-1;j++)
		{
			sum=sum+(((double)j/(double)i)*(C_ref[j]*A[i-j]));
		}
		C_ref[i]=A[i]+sum;	
	}
	return C_ref;
}

double *Apply_window(double* W_ref,double *C)			//function to apply the raised sine window in the Ci values calculated.
{
	int i=0;
	double x;
	for(i=1;i<=12;i++)
	{
		x=(3.14*i)/12;
		W_ref[i]=C[i]*(1+sin(x));
	}
	return W_ref;
}

void Print_accuracy(char c,int p)
{
	printf("--------->  ACCURACY  FOR  VOWEL  '%c'  is  %f \n\n\n\n",c,((p*10.0)));

}


double Compute_dc_shift()			//function to calculate the DC SHIFT
{
	FILE *fp;
	char ch;
	int count=0;
	double value=0,sum=0.0;
	double shift=0.0;
	fp=fopen("dc_shift_file.txt","r");      //opens the file that contain the value of silence

	while(!feof(fp))
	{
		fscanf(fp,"%lf",&value);			//reading values from the file
		sum=sum+value;						//adding up all the valeus of the file
	}

	fseek(fp,0,SEEK_SET);					//setting pointer to the initial position 0
	
	for(ch=getc(fp);ch!=EOF; ch=getc(fp))
	{
		if(ch=='\n')
			count++;							//counting the number of lines present the file
	}
	fclose(fp);

	shift=(sum/count);			//averaging the sum out

	return shift;
}

//Function to generate the reference files based on the training data
void reference_files()
{
	printf("\n  ------> REFERENCE FILES CREATED ARE:\n\n");

	char vowels_to_check[5]={ 'a','e','i','o','u'};

	for(int vowel=1;vowel<=5;vowel++)   //loop iterating over each vowel.
	{
		double reference[11][6][13];      //it contains all the ci values for 10 records 5 frames 12 sample ci values.
		double frame_array[5][321]={0};   //it contains the 320 samples for each frame.

		int count=0,index_max=0,k=0,i=0,j=0;;
		double max=0.0,value=0.0;

		char myfile[25];            //to store the file namne.
		FILE *file_pointer1;		//file pointer to point to the file.

		double R_ref[13];
		double A_ref[13];
		double C_ref[13];
		double W_ref[13];

		int record=0,frame=0,sample=0;

		double dc_shift=Compute_dc_shift();		//computing the dc shift 

		for(record=1;record<=10;record++)    //loop iterating over 10 recording  for each vowel
		{
			sprintf(myfile,"214101014_%c_%d.txt",vowels_to_check[vowel-1],record);   //generating the appropriate file name
			file_pointer1=fopen(myfile,"r");				//open the file and file pointe points to that file
			count=0;
			max=0;
			
			while(!feof(file_pointer1))
			{
				fscanf(file_pointer1,"%lf",&value);
				count++;
				if(value>max)
				{
					max=value;
					index_max=count;               //finding the index of maximum element in file.
				}
			}
			fseek(file_pointer1,0,SEEK_SET);

			int x=index_max-800;

			while(x--)
			{
				fscanf(file_pointer1,"%lf",&value);
				continue;
			}
			//consider 5 frames of 320 samples from 800 previous samples of maximum index and 800 samples after samples of maximum index. 
			for(i=0;i<5;i++)
			{
				for(j=0;j<320;j++)
				{
					fscanf(file_pointer1,"%lf",&frame_array[i][j]);
				}
			}
			
			fclose(file_pointer1);
			
			//Normalising the data file and applying the DC SHIFT
			for(i=0;i<5;i++)
			{
				for(j=0;j<320;j++)
				{
					frame_array[i][j]=((frame_array[i][j]-dc_shift)*5000.0)/max;
				}
			}

			for(frame=1;frame<=5;frame++)
			{
				double temp[320];
				for(int i=0;i<320;i++)
				{
					temp[i]=frame_array[frame][i];                //store the 320 samples pf each frame in a sample array to calculate the Ri,Ci and Ai values
				}

				double *R=Compute_Ri(temp,R_ref);                //compute teh value of Ri
				double *A=Compute_Ai(A_ref,R);					//compute teh value of Ai
				double *C=Compute_Ci(C_ref,A,R);				//compute teh value of Ci
				double * W=Apply_window(W_ref,C);				//Apply the raised sine window on the Ci values.


				for(sample=1;sample<=12;sample++)
				{
					reference[record][frame][sample]=W[sample];			//storing the ci values for 10 records 5 frames 12 sample ci values.
					
				}
			}
		}
	
		double ci_sum=0.0;
		double array_ci[6][13]={0.0};

		//Averaging the Ci values of all the files to get the 60 ci values
		for(int i=1;i<=5;i++)
		{
			for(int j=1;j<=12;j++)
			{
				ci_sum=0.0;
				for(int k=1;k<=10;k++)
				{
					ci_sum+=reference[k][i][j];
				}
				ci_sum=ci_sum/10;
				array_ci[i][j]=ci_sum;
			}
		}

		
		FILE *fp;
		char file_name[25];
		sprintf(file_name,"ref_%c.txt",vowels_to_check[vowel-1]);
		printf("\t FILE %d--> %s\n",vowel,file_name);
		fp=fopen(file_name,"w");  

		//Store all the calculated Ci values into the reference file.
		for(int i=1;i<=5;i++)
		{
			for(int j=1;j<=12;j++)
			{
				fprintf(fp,"%lf\n",array_ci[i][j]);
			}
		}

		fclose(fp);
	}
}

void testing_files()           //Function to generate the testing files and then compare it with the reference file and find the prediction of vowel and print the accuracy.
{
	printf("\n  ------> TEST FILES CREATED ARE:\n\n");
	
	char vowels_to_check[5]={ 'a','e','i','o','u'};
	double tokhura_weights[12]={1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0};

	for(int vowel=1;vowel<=5;vowel++)		//loop to iterate over the 5 vowels
	{
		double reference[6][13];			//store all the 60 ci values
		double frame_array[5][320]={0};		//store 320 samples of each frame

		int count=0,index_max=0;
		double max=0.0,value=0.0;
		int k=0,i=0,j=0;
		
		char myfile[25],file_name[25];					//to store the file namne.
		FILE *file_pointer1;							//file pointer to point to the file.

		double R_ref[13];
		double A_ref[13];
		double C_ref[13];

		int record=0,frame=0,sample=0;

		double dc_shift=Compute_dc_shift();			//compute dc shift
		
		int predict_accuracy=0;					//store prediction value

		for(record=1;record<=10;record++)		//loop to iterate over 10 recordings
		{
			sprintf(myfile,"214101014_%c_%d.txt",vowels_to_check[vowel-1],record+10);
			file_pointer1=fopen(myfile,"r");
			count=0;
			max=0;
		
			while(!feof(file_pointer1))
			{
				fscanf(file_pointer1,"%lf",&value);
				count++;
				if(value>max)
				{
					max=value;
					index_max=count;               //finding the index of maximum element in file.
				}
			}
			fseek(file_pointer1,0,SEEK_SET);

			int x=index_max-800;

			while(x--)
			{
				fscanf(file_pointer1,"%lf",&value);
				continue;
			}
			//consider 5 frames of 320 samples from 800 previous samples of maximum index and 800 samples after samples of maximum index. 
			for(i=0;i<5;i++)
			{
				for(j=0;j<320;j++)
				{
					fscanf(file_pointer1,"%lf",&frame_array[i][j]);
				}
			}

			fclose(file_pointer1);

			//Normalising and applying dc shift to the data
			for(i=0;i<5;i++)
			{
				for(j=0;j<320;j++)
				{
					frame_array[i][j]=((frame_array[i][j]-dc_shift)*5000.0)/max;
				}
			}

			for(frame=0;frame<5;frame++)
			{
				double temp[320];
				for(int i=0;i<320;i++)
				{
					temp[i]=frame_array[frame][i];			//store the 320 samples pf each frame in a sample array to calculate the Ri,Ci and Ai values
				}

				double *R=Compute_Ri(temp,R_ref);			//calculate the Ri value
				double *A=Compute_Ai(A_ref,R);				//calculate the Ai value
				double *C=Compute_Ci(C_ref,A,R);			//calculate the Ci value
				double W_ref[13];
				double * W=Apply_window(W_ref,C);			//apply raised sine window on ci values;

				for(sample=0;sample<12;sample++)
				{
					reference[frame][sample]=W[sample+1];		//to store the ci values calculated
				}
			}
			
			FILE *fp;
			char file_name[25];
			sprintf(file_name,"test_%c.txt",vowels_to_check[vowel-1]);
			printf("\t FILE %d.%d---> %s for recording %d\n",vowel,record,file_name,record);
			fp=fopen(file_name,"w");
			
			//to store the ci values in the test file created based on the testing data
			for(int i=0;i<5;i++)
			{
				for(int j=0;j<12;j++)
				{
					fprintf(fp,"%lf\n",reference[i][j]);
				}
			}
			fclose(fp);
			
			//printf("\tPREDICTION FOR THE VOWELS\n\n");

			//Calculating tokhura distance
			double tokhura_distance=DBL_MAX,final_distance=0.0,training_data=0.0,test_data=0.0;
			char detected_vowel;
			FILE *fp_r,*fp_t;    // pointers to point to the reference file and test file

			for(int inside=0;inside<5;inside++)
			{
				sprintf(file_name,"ref_%c.txt",vowels_to_check[inside]);
				sprintf(myfile,"test_%c.txt",vowels_to_check[vowel-1]);

				fp_r=fopen(file_name,"r");		//open the reference file
				fp_t=fopen(myfile,"r");			//open the test file

				for(int i=0;i<5;i++)
				{
					for(int j=0;j<12;j++)
					{
						fscanf(fp_r,"%lf",&training_data);
						fscanf(fp_t,"%lf",&test_data);

						double x=test_data-training_data;
						final_distance=final_distance+((x*x)*tokhura_weights[j]);         //calculating tokhura distance
					}
				}
				final_distance=final_distance/5.0;
				
				if(final_distance<=tokhura_distance)
				{
					tokhura_distance=final_distance;
					detected_vowel=vowels_to_check[inside];
				}
			}
			printf("\t\t Minimum Tokhura Distance for detected vowel '%c' : %lf\n\n",detected_vowel,tokhura_distance);
			if(detected_vowel==vowels_to_check[vowel-1])
			{
				predict_accuracy++;					//adding to the prediction count
			}
			
		}

		Print_accuracy(vowels_to_check[vowel-1],predict_accuracy);
		
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	int i=0,j=0;
	double value=0.0;
	double sample_data[320];
	double R_ref[13];           //Referenece array to store values of Ri.
	double A_ref[13];			//Referenece array to store values of Ai.
	double C_ref[13];			//Referenece array to store values of Ci.
	
	FILE *file_pointer;			//File pointer to access the data stored inside the file.
	
	file_pointer=fopen("test_new.txt","r");    //open the test file for validation of the Ri,Ai and Ci values.
	if(file_pointer==NULL)
	{ 8
		printf("Error in opening the file");     //Error message when the file does not contain any data to read.
		return (-1);
	}
	while(!feof(file_pointer))							// loop to read each value stored in the file 
	{
		fscanf(file_pointer,"%lf",&sample_data[i]);     // read values one by one
		i++;
	}
	double* R=Compute_Ri(sample_data,R_ref);       //function to calculate Ri
	double* A=Compute_Ai(A_ref,R);					//function to calculate Ai
	double* C=Compute_Ci(C_ref,A,R);				//function to calculate Ci

	printf("\n\t\t\tSPEECH PROCESSING ASSIGNMENT-3\n\t\t\tVOWEL RECOGNITION");
	printf("\n\n\t Name: Darshika Verma\n\t Roll_no: 214101014\n");
	printf("\n  ------> VALIDATION OF R_i,A_i and C_i VALUES\n\n");
	for(i=1;i<=12;i++)
	{
		printf("\tC[%d]=%lf\tA[%d]=%lf\t\tC[%d]=%lf\n\n",i,R[i],i,A[i],i,C[i]);
	}

	reference_files();		//funtion to generate the reference files using the training data
	
	testing_files();		//function to generate the test files using the testing data and give the prediction for accuracy of each vowel
	
	system("PAUSE");		//function used to stop the output console screen.	
	return 0;
}

