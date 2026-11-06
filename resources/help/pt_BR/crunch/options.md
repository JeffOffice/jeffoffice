# Alterar as configurações do JeffCrunch
<!-- keywords: edição, preenchimento automático, alça de preenchimento, arrastar e soltar, casas decimais, opções, configurações, preferências, estatísticas de uso, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, excluir estatísticas, atualizações, sobre, editar na célula, cor das linhas de grade, zoom padrão, largura padrão, privacidade, informações pessoais -->

Suas configurações estão na janela **Opções do JeffCrunch**. Clique em **Arquivo > Opções** para abri-la. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Edição

A página **Edição** altera o modo como a digitação funciona nas células.

- **Após pressionar Enter, mover a seleção** define para onde a seleção vai quando você conclui uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lugar nenhum.
- **Oferecer para concluir um texto a partir das células acima e abaixo** é chamado de AutoPreenchimento. Quando você digita as primeiras letras de um texto e apenas um texto dessa coluna começa com elas, o JeffCrunch mostra o restante, selecionado. Pressione **Enter** para mantê-lo. Continue digitando para ignorá-lo, ou pressione **Backspace** para removê-lo. Ele considera somente o bloco de células ao lado da célula em que você está digitando, e somente textos, não números.
- **Usar a alça de preenchimento e arrastar células para mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrasto pela borda da seleção. Desative-o se você move células sem querer. Copiar e Colar continuam funcionando.
- **Mostrar zero em células que têm valor zero (esta planilha)** ativa ou desativa os zeros na planilha que você está vendo. Quando está desativado, uma célula que contém 0 parece vazia, mas continua contendo o 0 e é somada como 0. A escolha é salva com a planilha.
- **Colocar um ponto decimal em números inteiros digitados** é chamado de casas decimais automáticas. Defina **Casas** como 2, e ao digitar 1234 será inserido 12,34. Defina como -2, e ao digitar 5 será inserido 500. Um número digitado com vírgula decimal é inserido como digitado.

- **Permitir edição diretamente nas células** vem ativado por padrão. Quando você o desativa, uma entrada digitada ou F2 vai para a barra de fórmulas, e a célula mostra o resultado somente quando você conclui a entrada.

## Opções de exibição

- **Cor das linhas de grade** é a cor das linhas entre as células na tela. É uma escolha apenas para a sua tela, não salva na pasta de trabalho.
- **Zoom de uma nova pasta de trabalho** define o tamanho das planilhas de uma nova pasta de trabalho, de 10% a 400%.
- **Largura padrão de coluna (esta planilha)** é a largura de todas as colunas desta planilha que não têm largura própria. É o mesmo que **Largura Padrão** na guia Página Inicial.

## Opções de privacidade

- **Remover meu nome e empresa das propriedades de uma pasta de trabalho sempre que ela for salva** limpa o autor, quem a salvou por último, a empresa e o gerente das propriedades da pasta de trabalho quando você salva. As propriedades são as informações que um arquivo guarda sobre si mesmo.

## Estatísticas de uso

**Enviar estatísticas de uso anônimas** permite compartilhar informações sobre como o JeffCrunch é usado. Anônimo significa que as informações não dizem nada sobre quem você é. Quando isso está ativado, o JeffCrunch envia essas informações uma vez por dia e no dia em que você atualiza: que é o JeffCrunch, sua versão, seu sistema operacional e idioma, quantas vezes ele foi iniciado, com que frequência cada comando é usado e que tipo de arquivo você abriu ou salvou. Se o JeffCrunch fechar inesperadamente, ele também envia um relatório de erro na próxima vez que for iniciado. O JeffCrunch nunca envia seus arquivos, os nomes deles, nem qualquer conteúdo, nem o que você digitou. Se isso estiver desativado, nenhum relatório de erro é enviado.

A escolha vale para todos os aplicativos do JeffOffice: altere-a aqui e ela muda em todos eles. O JeffCrunch pergunta sobre isso na primeira vez que é iniciado, em uma janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de seleção, abre nosso aviso de privacidade em jeffoffice.com/privacy. Ele informa o que é enviado, para onde vai e por quanto tempo guardamos os dados. Mantemos as estatísticas por 24 meses.
- **Número de instalação** é mostrado abaixo da caixa de seleção. É um número aleatório criado para a sua cópia do JeffOffice, igual em todos os aplicativos do JeffOffice. Ele não diz nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número tiver sido criado ainda, aparece **nenhum**.
- Para que as estatísticas que já temos sejam excluídas, envie seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos saber quais registros são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Assim, nada é contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
