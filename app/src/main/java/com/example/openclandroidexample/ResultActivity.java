package com.example.openclandroidexample;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.cardview.widget.CardView;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import java.util.Objects;

public class ResultActivity extends AppCompatActivity {

    private CardView successCard, errorCard;
    private TextView resultMatrixTextView, errorMessageTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_result);

        Toolbar toolbar = findViewById(R.id.toolbar);
        //setSupportActionBar(toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);


        successCard = findViewById(R.id.success_card);
        errorCard = findViewById(R.id.error_card);
        resultMatrixTextView = findViewById(R.id.result_matrix);
        errorMessageTextView = findViewById(R.id.error_message);

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            if (extras.containsKey("result_matrix")) {
                String resultMatrix = (String) extras.getSerializable("result_matrix");
                displaySuccess(resultMatrix);
            } else if (extras.containsKey("error_message")) {
                String errorMessage = extras.getString("error_message");
                displayError(errorMessage);
            }
        }
    }

    private void displaySuccess(String matrixString) {
        successCard.setVisibility(View.VISIBLE);
        errorCard.setVisibility(View.GONE);
        resultMatrixTextView.setText(matrixString);
    }

    private void displayError(String message) {
        successCard.setVisibility(View.GONE);
        errorCard.setVisibility(View.VISIBLE);
        errorMessageTextView.setText(message);
    }


    @Override
    public boolean onSupportNavigateUp() {
        onBackPressed();
        return true;
    }
}
