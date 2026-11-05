# Alterar as definições do JeffCrunch
<!-- keywords: edição, preenchimento automático, alça de preenchimento, arrastar e largar, casas decimais, opções, definições, preferências, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de -->

As suas definições estão na janela **Opções do JeffCrunch**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Edição

A página **Edição** altera a forma como a escrita nas células funciona.

- **Depois de premir Enter, mover a seleção** indica para onde vai a seleção quando termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lado nenhum.
- **Sugerir a conclusão de um texto a partir das células acima e abaixo** chama-se Preenchimento Automático. Quando escreve as primeiras letras de um texto e só um texto dessa coluna começa por elas, o JeffCrunch mostra o resto, selecionado. Prima **Enter** para o manter. Continue a escrever para o ignorar, ou prima **Retrocesso** para o retirar. Este recurso analisa apenas o bloco de células ao lado daquela em que está a escrever, e apenas textos, não números.
- **Utilizar a alça de preenchimento e arrastar células para mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastar do limite da seleção. Desative-o se mover células por engano. Copiar e Colar continuam a funcionar.
- **Colocar uma vírgula decimal em números inteiros introduzidos** chama-se casas decimais automáticas. Defina **Casas** como 2, e ao escrever 1234 é introduzido 12,34. Defina-as como -2, e ao escrever 5 é introduzido 500. Um número escrito com vírgula decimal é introduzido tal como o escreveu.

## Estatísticas de utilização

**Enviar estatísticas de utilização anónimas** permite partilhar dados sobre a forma como o JeffCrunch é utilizado. Anónimo significa que os dados não revelam quem é. Quando esta opção está ativada, o JeffCrunch envia estes dados uma vez por dia e no dia em que atualizar: que se trata do JeffCrunch, a respetiva versão, o seu sistema operativo e idioma, quantas vezes foi iniciado, a frequência de utilização de cada comando e o tipo de ficheiro que abriu ou guardou. Se o JeffCrunch fechar inesperadamente, envia também um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia os seus ficheiros, os respetivos nomes ou qualquer conteúdo, nem aquilo que escreveu. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha aplica-se a todas as aplicações JeffOffice: se a alterar aqui, fica alterada em cada uma delas. O JeffCrunch pergunta por esta opção na primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Nele se indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não revela nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, aparece **nenhum**.
- Para pedir a eliminação das estatísticas que já temos, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos identificar que registos são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Nada é então contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
