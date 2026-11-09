# Alterar as configurações do JeffCrunch
<!-- keywords: recuperação, recuperar trabalho não salvo, recuperação automática, revisão de texto, edição, preenchimento automático, alça de preenchimento, arrastar e soltar, casas decimais, opções, configurações, preferências, estatísticas de uso, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, excluir estatísticas, atualizações, sobre, editar na célula, cor das linhas de grade, zoom padrão, largura padrão, privacidade, informações pessoais -->

Suas configurações estão na janela **Opções do JeffCrunch**. Clique em **Arquivo > Opções** para abri-la. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Edição

A página **Edição** altera como a digitação nas células funciona.

- **Após pressionar Enter, mover a seleção** define para onde a seleção vai quando você termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lugar nenhum.
- **Sugerir a conclusão de um texto a partir das células acima e abaixo** é chamado de Preenchimento Automático de texto. Quando você digita as primeiras letras de um texto e apenas um texto dessa coluna começa com elas, o JeffCrunch mostra o restante, selecionado. Pressione **Enter** para mantê-lo. Continue digitando para ignorá-lo ou pressione **Backspace** para removê-lo. Ele considera apenas o bloco de células ao lado daquela em que você está digitando e apenas textos, não números.
- **Usar a alça de preenchimento e arrastar células para mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrasto pela borda da seleção. Desative se você move células sem querer. Copiar e Colar continuam funcionando.
- **Mostrar zero em células com valor zero (esta planilha)** ativa ou desativa os zeros na planilha que você está vendo. Quando está desativado, uma célula com 0 parece vazia, mas continua contendo o 0 e é somada como 0. A escolha é salva com a planilha.
- **Colocar vírgula decimal em números inteiros digitados** é chamado de casas decimais automáticas. Defina **Casas** como 2, e digitar 1234 insere 12,34. Defina como -2, e digitar 5 insere 500. Um número digitado com vírgula decimal é inserido como foi digitado.

- **Permitir edição diretamente nas células** vem ativado por padrão. Quando você desativa, uma entrada digitada ou F2 vai para a barra de fórmulas, e a célula mostra o resultado somente quando você conclui a entrada.

## Opções de salvamento

**Salvar informações de recuperação automática a cada** define com que frequência o JeffCrunch guarda uma cópia de uma pasta de trabalho com alterações que você ainda não salvou, de 1 a 120 minutos. A cópia é um arquivo .xlsx em uma pasta que o JeffCrunch mantém para essa finalidade, feita em segundo plano para que você possa continuar digitando. Ela é removida quando você salva ou fecha a pasta de trabalho. Se o JeffCrunch fechar inesperadamente, ele oferece as cópias na próxima vez que for iniciado. Você também pode clicar em **Arquivo > Abrir > Recuperar Trabalho Não Salvo**. Uma pasta de trabalho protegida por senha não é copiada, pois a cópia não estaria protegida.

## Opções de revisão de texto

A página **Revisão de Texto** é compartilhada pelos aplicativos do JeffOffice. Ela define se a ortografia é verificada enquanto você digita em uma célula (desative **Verificar ortografia ao digitar** para eliminar os sublinhados vermelhos ondulados; **Ortografia** na guia Revisão continua funcionando), se palavras em letras maiúsculas e palavras com números são ignoradas na verificação ortográfica, se a AutoCorreção corrige palavras enquanto você digita, se aspas retas viram aspas curvas, se travessões, frações e ordinais são alterados e se um endereço da web vira um link. Consulte [Ortografia](notes-links) e [Digitar em células](edit-cells).

## Opções de exibição

- **Cor das linhas de grade** é a cor das linhas entre as células na sua tela. É uma escolha apenas para a sua tela, não salva na pasta de trabalho.
- **Zoom de uma nova pasta de trabalho** define o tamanho com que as planilhas de uma nova pasta de trabalho aparecem, de 10% a 400%.
- **Largura padrão de coluna (esta planilha)** é a largura de cada coluna desta planilha que não tem largura própria. É o mesmo que **Largura Padrão** na guia Página Inicial.

## Opções de privacidade

- **Remover meu nome e empresa das propriedades de uma pasta de trabalho a cada vez que ela for salva** limpa o autor, quem salvou por último, a empresa e o gerente das propriedades da pasta de trabalho ao salvá-la. As propriedades são as informações que um arquivo guarda sobre si mesmo.

## Estatísticas de uso

**Enviar estatísticas de uso anônimas** permite compartilhar dados sobre como o JeffCrunch é usado. Anônimas significa que os dados não dizem nada sobre quem você é. Quando isso está ativado, o JeffCrunch envia esses dados uma vez por dia e no dia em que você atualiza: que é o JeffCrunch, sua versão, seu sistema operacional e idioma, quantas vezes ele foi iniciado, com que frequência cada comando é usado e que tipo de arquivo você abriu ou salvou. Se o JeffCrunch fechar inesperadamente, ele também envia um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia seus arquivos, os nomes deles, nem qualquer conteúdo, nem o que você digitou. Se isso estiver desativado, nenhum relatório de falha é enviado.

A escolha vale para todos os aplicativos do JeffOffice: se você a alterar aqui, ela muda em cada um deles. O JeffCrunch pergunta sobre isso na primeira vez que é iniciado, em uma janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de seleção, abre nosso aviso de privacidade em jeffoffice.com/privacy. Ele informa o que é enviado, para onde vai e por quanto tempo guardamos os dados. Mantemos as estatísticas por 24 meses.
- **Número de instalação** aparece abaixo da caixa de seleção. É um número aleatório gerado para a sua cópia do JeffOffice, o mesmo em todos os aplicativos do JeffOffice. Ele não diz nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número tiver sido gerado ainda, aparece **nenhum**.
- Para excluir as estatísticas que já temos, envie seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos identificar quais registros são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Nada é então contado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
