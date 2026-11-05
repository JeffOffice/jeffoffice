# Alterar as configurações do JeffCrunch
<!-- keywords: edição, preenchimento automático, alça de preenchimento, arrastar e soltar, casas decimais, opções, configurações, preferências, estatísticas de uso, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, excluir estatísticas, atualizações, sobre -->

As suas configurações estão na janela **Opções do JeffCrunch**. Clique em **Arquivo > Opções** para abri-la. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Edição

A página **Edição** altera o modo como a digitação nas células funciona.

- **Após pressionar Enter, mover a seleção** define para onde a seleção vai quando você termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lugar nenhum.
- **Sugerir a conclusão de um texto a partir das células acima e abaixo** é chamado de AutoPreenchimento. Quando você digita as primeiras letras de um texto e apenas um texto dessa coluna começa com elas, o JeffCrunch mostra o restante, selecionado. Pressione **Enter** para mantê-lo. Continue digitando para ignorá-lo ou pressione **Backspace** para removê-lo. Ele considera apenas o bloco de células ao lado daquela em que você está digitando, e apenas textos, não números.
- **Usar a alça de preenchimento e arrastar células para mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastar pela borda da seleção. Desative se você mover células por engano. Copiar e Colar continuam funcionando.
- **Inserir uma vírgula decimal em números inteiros digitados** é chamado de casas decimais automáticas. Defina **Casas** como 2, e digitar 1234 insere 12,34. Defina como -2, e digitar 5 insere 500. Um número digitado com vírgula decimal é inserido exatamente como foi digitado.

## Estatísticas de uso

**Enviar estatísticas de uso anônimas** permite compartilhar dados sobre como o JeffCrunch é usado. Anônimo significa que os dados não dizem nada sobre quem você é. Quando esta opção está ativada, o JeffCrunch envia esses dados uma vez por dia e no dia em que você atualizar: que é o JeffCrunch, a versão, o seu sistema operacional e idioma, quantas vezes ele foi iniciado, com que frequência cada comando é usado e que tipo de arquivo você abriu ou salvou. Se o JeffCrunch fechar inesperadamente, ele também envia um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia os seus arquivos, os nomes deles nem qualquer conteúdo, nem o que você digitou. Se esta opção estiver desativada, nenhum relatório de falha é enviado.

A escolha vale para todos os aplicativos do JeffOffice: alterá-la aqui a altera em cada um deles. O JeffCrunch pergunta sobre isso na primeira vez que é iniciado, na janela **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de seleção, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Ele informa o que é enviado, para onde vai e por quanto tempo o guardamos. Mantemos as estatísticas por 24 meses.
- **Número de instalação** é exibido abaixo da caixa de seleção. É um número aleatório gerado para a sua cópia do JeffOffice, igual em todos os aplicativos do JeffOffice. Ele não diz nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número tiver sido gerado ainda, aparece **nenhum**.
- Para pedir a exclusão das estatísticas que já temos, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos saber quais registros são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Assim, nada é contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
